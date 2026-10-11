#include "base/http.h"
#include "base/version.h"
#include <cstdio>
#include <curl/curl.h>
#include <fmt/format.h>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <zlib.h>

namespace http {

static const std::string UA = fmt::format("ZQuestClassic/{}", getVersionString());
static const std::string VERSION_HEADER = fmt::format("ZC-Version: {}", getVersionString());

bool http_response::success() const
{
	return status_code >= 200 && status_code < 300;
}

static size_t _write_callback(char *contents, size_t size, size_t nmemb, void *userp)
{
	size_t total_size = size * nmemb;
	std::string* str = (std::string*)userp;
	str->insert(str->end(), contents, contents + total_size);
	return total_size;
}

// Cleans up the curl handle, and sets the response's status code.
static expected<http_response, std::string> finish_request(CURL* curl_handle, CURLcode res, http_response response)
{
	// CURLINFO_RESPONSE_CODE writes a long.
	long status_code = 0;
	if (res == CURLE_OK)
		curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &status_code);
	curl_easy_cleanup(curl_handle);

	if (res != CURLE_OK)
	{
		std::string error = fmt::format("curl: {}", curl_easy_strerror(res));
		fmt::println(stderr, "{}", error);
		return make_unexpected(error);
	}

	response.status_code = status_code;
	return std::move(response);
}

expected<http_response, std::string> get(std::string url)
{
	http_response response{};

	CURL *curl_handle = curl_easy_init();
	if (!curl_handle)
		return make_unexpected("curl: failed to initialize");

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, VERSION_HEADER.c_str());

	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response.body);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, UA.c_str());
	CURLcode res = curl_easy_perform(curl_handle);

	curl_slist_free_all(headers);

	return finish_request(curl_handle, res, std::move(response));
}

static bool compress_file_to_gzip(const fs::path& input_path, const fs::path& output_path)
{
	FILE* in = fopen(input_path.string().c_str(), "rb");
	if (!in)
		return false;

	gzFile out = gzopen(output_path.string().c_str(), "wb");
	if (!out)
	{
		fclose(in);
		return false;
	}

	char buffer[8192];
	int bytes_read;
	bool success = true;
	while ((bytes_read = fread(buffer, 1, sizeof(buffer), in)) > 0)
	{
		if (gzwrite(out, buffer, bytes_read) != bytes_read)
		{
			success = false;
			break;
		}
	}
	if (ferror(in))
		success = false;

	fclose(in);
	if (gzclose(out) != Z_OK)
		success = false;

	return success;
}

expected<http_response, std::string> upload(std::string url, fs::path path)
{
	http_response response{};

	fs::path compressed_path = path.string() + ".gz";
	std::error_code ec;
	if (!compress_file_to_gzip(path, compressed_path))
	{
		fs::remove(compressed_path, ec);
		return make_unexpected("failed to compress file");
	}

	FILE* fd = fopen(compressed_path.string().c_str(), "rb");
	if (!fd)
	{
		fs::remove(compressed_path, ec);
		return make_unexpected("can't read compressed file");
	}

	struct stat file_info;
	if (fstat(fileno(fd), &file_info) != 0)
	{
		fclose(fd);
		fs::remove(compressed_path, ec);
		return make_unexpected("can't stat file");
	}

	CURL *curl_handle = curl_easy_init();
	if (!curl_handle)
	{
		fclose(fd);
		fs::remove(compressed_path, ec);
		return make_unexpected("curl: failed to initialize");
	}

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Encoding: gzip");
	headers = curl_slist_append(headers, VERSION_HEADER.c_str());

	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response.body);
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, fd);
	curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, UA.c_str());

	CURLcode res = curl_easy_perform(curl_handle);

	fclose(fd);
	curl_slist_free_all(headers);
	fs::remove(compressed_path, ec);

	return finish_request(curl_handle, res, std::move(response));
}

}

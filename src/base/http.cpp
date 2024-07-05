#include "base/http.h"
#include "base/version.h"
#include <cstdio>
#include <curl/curl.h>
#include <fmt/format.h>
#include <string>
#include <sys/stat.h>

namespace http {

static const std::string UA = fmt::format("ZQuestClassic/{}", getVersionString());

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

expected<http_response, std::string> get(std::string url)
{
	http_response response{};

	CURL *curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response.body);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, UA.c_str());
	CURLcode res = curl_easy_perform(curl_handle);

	if (res != CURLE_OK)
	{
		std::string error = fmt::format("curl: {}", curl_easy_strerror(res));
		fmt::println(stderr, "{}", error);
		return make_unexpected(error);
	}

	curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response.status_code);
	curl_easy_cleanup(curl_handle);

	return response;
}

expected<http_response, std::string> upload(std::string url, fs::path path)
{
	http_response response{};

	FILE* fd = fopen(path.string().c_str(), "rb");
	if (!fd)
		return make_unexpected("can't read file");

	struct stat file_info;
	if (fstat(fileno(fd), &file_info) != 0)
	{
		fclose(fd);
		return make_unexpected("can't stat file");
	}

	CURL *curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, _write_callback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response.body);
	curl_easy_setopt(curl_handle, CURLOPT_READDATA, fd);
	curl_easy_setopt(curl_handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, UA.c_str());
	CURLcode res = curl_easy_perform(curl_handle);
	fclose(fd);

	if (res != CURLE_OK)
	{
		std::string error = fmt::format("curl: {}", curl_easy_strerror(res));
		fmt::println(stderr, "{}", error);
		return make_unexpected(error);
	}

	curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response.status_code);
	curl_easy_cleanup(curl_handle);

	return response;
}

}

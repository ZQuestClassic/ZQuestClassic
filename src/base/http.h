#ifndef BASE_HTTP_H_
#define BASE_HTTP_H_

#include "base/expected.h"
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace http {

struct http_response
{
	int status_code;
	std::string body;

	bool success() const;
};

expected<http_response, std::string> get(std::string url);
// Sends a PUT request.
expected<http_response, std::string> upload(std::string url, fs::path path);

};

#endif

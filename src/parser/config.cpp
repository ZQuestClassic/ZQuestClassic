#include "parser/config.h"

#include <string>
#include <filesystem>
#include <fstream>
#include <map>

#include "base/util.h"
#include "base/headers.h"

namespace fs = std::filesystem;

static std::map<std::string, std::string> base_config;
static std::map<std::string, std::string> user_config;
void zconsole_db(const char *format,...);
static inline string& trim(string& str)
{
	str.erase(str.find_last_not_of(" \t\r\n")+1);
	str.erase(0,str.find_first_not_of(" \t\r\n"));
	return str;
}
static bool load_config(std::map<std::string, std::string>& config, std::string path)
{
	config.clear();
	std::ifstream file(path);
	if (!file.is_open())
		return false;

	std::string section;
	std::string line;
	while (util::portable_get_line(file, line))
	{
		if (line.empty())
			continue;
		
		if (line[0] == '[')
		{
			section = line;
			continue;
		}

		if (section != "[Compiler]")
			continue;
		
		std::vector<std::string> tokens;
		util::split(line, tokens, '=');
		if (tokens.size() != 2)
			continue;
		config[trim(tokens[0])] = trim(tokens[1]);
	}

	return true;
}

bool zscript_load_base_config(std::string path)
{
	return load_config(base_config, path);
}

bool zscript_load_user_config(std::string path)
{
	return load_config(user_config, path);
}

std::string zscript_get_config_string(std::string key, std::string def_value)
{
	auto it = user_config.find(key);
	if (it != user_config.end())
		return it->second;
	
	it = base_config.find(key);
	if (it != base_config.end())
		return it->second;
	
	return def_value;
}

int zscript_get_config_int(std::string key, int def_value)
{
	auto it = user_config.find(key);
	if (it != user_config.end())
		return std::stoi(it->second);
	
	it = base_config.find(key);
	if (it != base_config.end())
		return std::stoi(it->second);
	return def_value;
}

double zscript_get_config_double(std::string key, double def_value)
{
	auto it = user_config.find(key);
	if (it != user_config.end())
		return std::stod(it->second);
	
	it = base_config.find(key);
	if (it != base_config.end())
		return std::stod(it->second);
	
	return def_value;
}

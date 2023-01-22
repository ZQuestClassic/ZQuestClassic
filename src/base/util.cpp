#include "base/util.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fmt/format.h>
#include <sys/stat.h>

using namespace std;

#define PATH_MODE		0755

namespace util
{
	void upperstr(string& str)
	{
		for(int32_t q = str.size() - 1; q >= 0; --q)
			str[q] = toupper(str[q]);
	}

	void lowerstr(string& str)
	{
		for(int32_t q = str.size() - 1; q >= 0; --q)
			str[q] = tolower(str[q]);
	}

	void trimstr(string& str)
	{
		str.erase(0, str.find_first_not_of("\t\n\v\f\r "));
		str.erase(str.find_last_not_of("\t\n\v\f\r ") + 1);
	}
	
	string cropPath(string filepath)
	{
		size_t lastslash = filepath.find_last_of("/");
		size_t lastbslash = filepath.find_last_of("\\");
		size_t last = (lastslash == string::npos) ? lastbslash : (lastbslash == string::npos ? lastslash : (lastslash > lastbslash ? lastslash : lastbslash));
		if(last != string::npos) filepath = filepath.substr(last+1);
		return filepath;
	}
	
	void replstr(std::string& str, std::string const& from, std::string const& to)
	{
		for(int32_t q = str.size() - from.size(); q >= 0; --q)
		{
			if(!str.compare(q,from.size(),from))
			{
				str = str.replace(q,from.size(),to);
				q -= from.size()-1;
			}
		}
	}
	
	std::string escape_characters(std::string const& str)
	{
		std::string temp = str;
		replstr(temp, "\\", "\\\\");
		replstr(temp, "\"", "\\\"");
		replstr(temp, "'", "\\'");
		replstr(temp, "\a", "\\a");
		replstr(temp, "\b", "\\b");
		replstr(temp, "\f", "\\f");
		replstr(temp, "\n", "\\n");
		replstr(temp, "\r", "\\r");
		replstr(temp, "\t", "\\t");
		replstr(temp, "\v", "\\v");
		return temp;
	}
	std::string disallow_escapes(std::string const& str)
	{
		std::string temp = str;
		replstr(temp, "\\\\", "\\");
		replstr(temp, "\\\"", "\"");
		replstr(temp, "\\'", "'");
		replstr(temp, "\\a", "");
		replstr(temp, "\\b", "");
		replstr(temp, "\\f", "");
		replstr(temp, "\\n", "");
		replstr(temp, "\\r", "");
		replstr(temp, "\\t", "");
		replstr(temp, "\\v", "");
		return temp;
	}
	std::string unescape_characters(std::string const& str)
	{
		std::string temp = str;
		replstr(temp, "\\\\", "\\");
		replstr(temp, "\\\"", "\"");
		replstr(temp, "\\'", "'");
		replstr(temp, "\\a", "\a");
		replstr(temp, "\\b", "\b");
		replstr(temp, "\\f", "\f");
		replstr(temp, "\\n", "\n");
		replstr(temp, "\\r", "\r");
		replstr(temp, "\\t", "\t");
		replstr(temp, "\\v", "\v");
		return temp;
	}
	
	void replchar(std::string& str, char from, char to)
	{
		for(int32_t q = str.size() - 1; q >= 0; --q)
		{
			if(str.at(q)==from)
				str[q] = to;
		}
	}
	
	void replchar(char* buf, char from, char to)
	{
		for(int32_t i = 0; buf[i]!=0; ++i)
		{
			if(buf[i]==from)
				buf[i] = to;
		}
	}
	
	void removechar(std::string& str, char to_delete)
	{
		size_t pos = str.find_first_of(to_delete);
		while(pos != std::string::npos)
		{
			str.erase(pos,1);
			pos = str.find_first_of(to_delete);
		}
	}
	
	string get_ext(string const& path)
	{
		string ext = std::filesystem::path(path).extension().string();
		lowerstr(ext);
		return ext;
	}
	
	static bool valid_single_dir(string const& path)
	{
		if(path.find_first_of("<>|?*&^$#\"") != string::npos) return false; //Contains invalid chars
		size_t nonslash_pos = path.find_last_not_of("/\\");
		if(nonslash_pos == string::npos) return false; //blank or all slashes
		if(path[0] == '/' || path[0] == '\\') return false; //multiple consecutive slashes
		if(path.find_first_not_of("./\\") == string::npos) return false; //empty dirname
		if(path.find("..") == 0) return false; //cannot begin with >1 dot
		if(path.find("...") != string::npos) return false; //cannot contain >2 consecutive dots
		return true;
	}
	
	bool valid_dir(string const& path)
	{
		size_t pos = path.find_first_not_of("/\\");
		if(pos == string::npos) return true;
		while(pos < path.length())
		{
			size_t next_slash = path.find_first_of("/\\",pos);
			if(next_slash == string::npos) break;
			if(!valid_single_dir(path.substr(pos,next_slash-pos))) return false;
			pos = next_slash+1;
		}
		return true;
	}
	
	bool valid_file(string const& path)
	{
		if(path.find_first_of("<>|?*&^$#\"") != string::npos) return false; //Contains invalid chars
		size_t last_slash_pos = path.find_last_of("/\\");
		if(last_slash_pos == string::npos) last_slash_pos = 0;
		else
		{
			if(!valid_dir(path.substr(0,last_slash_pos))) return false;
			++last_slash_pos;
		}
		if(last_slash_pos == path.length() - 1) return false; //Ends in slash; empty filename
		string fname = path.substr(last_slash_pos);
		if(fname.find_first_of(":") != string::npos) return false; //invalid char
		if(fname.find_first_not_of(".") == string::npos) return false; //empty filename
		if(fname.find("..") == 0) return false; //cannot begin with >1 dot
		if(fname.find("...") != string::npos) return false; //cannot contain >2 consecutive dots
		return true;
	}
	
	void regulate_path(char* buf)
	{
		for(int32_t q = 0; buf[q]; ++q)
		{
			if (buf[q] == WRONG_PATH_SLASH) buf[q] = PATH_SLASH;
		}
	}
	
	void regulate_path(string& buf)
	{
		for(int32_t q = 0; buf[q]; ++q)
		{
#ifdef _WIN32
			if (buf[q] == '/') buf[q] = '\\';
#else
			if (buf[q] == '\\') buf[q] = '/';
#endif
		}
	}
	
	int32_t do_mkdir(const char* path, int32_t mode)
	{
#ifdef _WIN32
		return _mkdir(path);
#else
		return mkdir(path,mode);
#endif
	}

	bool create_path(const char* path)
	{
		while((path[0] == '/' || path[0] == '\\') && path[0]) ++path; //trim leading slashes
		char buf[2048] = {0};
		int32_t q = 0;
		int32_t last_slash = -1;
		for(; path[q] && q < 2048; ++q)
		{
			buf[q] = path[q];
			if(path[q] == '/' || path[q] == '\\')
			{
				string strpath(buf+last_slash+1);
				last_slash = q;
				if(strpath == ". /" || strpath == ".\\")
					continue;
				if(strpath.find_first_of(":") != string::npos)
					continue; //Non-creatable; ex "C:\"
				if(!valid_single_dir(strpath))
				{
					return false; //Failure; invalid path
				}
				
				struct stat info;
				if(stat( buf, &info ) != 0)
				{
					if (do_mkdir(buf, PATH_MODE) != 0 && errno != EEXIST)
					{
						return false; //Failure; could not create
					}
				}
				else if((info.st_mode & S_IFDIR)==0)
				{
					return false; //Hit failure; exists, but is not dir.
				}
			}
		}
		return q < 2048;
	}

	static void reverse( char *s )
	{
		if ( *s )
		{
			for ( char *first = s, *last = s + strlen( s ); first < --last; ++first )
			{
				char c = *first;
				*first = *last;
				*last = c;
			}
		}
	}
	
	char* zc_itoa(int32_t value, char* str, int32_t base)
	{
#ifdef _WIN32
		return _itoa(value, str, base);
#else
		static char dig[] =
			"0123456789"
			"abcdefghijklmnopqrstuvwxyz";
		int32_t n = 0, neg = 0;
		uint32_t v;
		char* p, *q;
		char c;
		if (base == 10 && value < 0) 
		{
			value = -value;
			neg = 1;
		}
		v = value;
		do 
		{
			str[n++] = dig[v%base];
			v /= base;
		} while (v);
		if (neg)
		{
			str[n++] = '-';
		}
		str[n] = '\0';
		reverse(str);
		return str;
#endif
	}
	
	int64_t zc_atoi64(const char *str)
	{
		int64_t val=0;
		bool neg = false;
		if(*str == '-')
		{
			neg = true;
			++str;
		}
		while(isdigit(*str))
		{
			val*=10;
			
			val += *str-'0';
			
			++str;
		}
		
		return neg ? -val : val;
	}
	int64_t zc_xtoi64(const char *hexstr)
	{
		int64_t val=0;
		bool neg = false;
		if(*hexstr == '-')
		{
			neg = true;
			++hexstr;
		}
		while(isxdigit(*hexstr))
		{
			val<<=4;
			
			if(*hexstr<='9')
				val += (*hexstr-'0');
			else val+= ((*hexstr)|0x20)-'a'+10;
			++hexstr;
		}
		
		return neg ? -val : val;
	}
	
	int32_t zc_xtoi(const char *hexstr)
	{
		int32_t val=0;
		bool neg = false;
		if(*hexstr == '-')
		{
			neg = true;
			++hexstr;
		}
		while(isxdigit(*hexstr))
		{
			val<<=4;
			
			if(*hexstr<='9')
				val += *hexstr-'0';
			else val+= ((*hexstr)|0x20)-'a'+10;
			
			++hexstr;
		}
		
		return neg ? -val : val;
	}
	
	int32_t ffparse2(const char *string) //bounds result safely between -214748.3648 and +214748.3647
	{
		char tempstring1[32] = {0};
		strcpy(tempstring1, string);
		
		char *ptr=strchr(tempstring1, '.');
		if(!ptr)
		{
			return vbound(atoi(tempstring1),-214748,214748)*10000;
		}
		
		int32_t ret=0;
		
		for(int32_t i=0; i<4; ++i)
		{
			tempstring1[strlen(string)+i]='0';
		}
		
		ptr=strchr(tempstring1, '.');
		*ptr=0;
		ret=vbound(atoi(tempstring1),-214748,214748)*10000;
		
		++ptr;
		char *ptr2=ptr;
		ptr2+=4;
		*ptr2=0;
		
		int32_t decval = abs(atoi(ptr));
		if(tempstring1[0] == '-')
		{
			if(ret == -2147480000)
				decval = vbound(decval, 0, 3648);
			ret-=decval;
		}
		else
		{
			if(ret == 2147480000)
				decval = vbound(decval, 0, 3647);
			ret+=decval;
		}
		
		return ret;
	}
	int32_t ffparseX(const char *string) //hex before '.', bounds result safely between -214748.3648 and +214748.3647
	{
		char tempstring1[32] = {0};
		strcpy(tempstring1, string);
		
		char *ptr=strchr(tempstring1, '.');
		if(!ptr)
		{
			return vbound(zc_xtoi(tempstring1),-214748,214748)*10000;
		}
		
		int32_t ret=0;

		strcpy(tempstring1, string);
		
		for(int32_t i=0; i<4; ++i)
		{
			tempstring1[strlen(string)+i]='0';
		}
		
		ptr=strchr(tempstring1, '.');
		*ptr=0;
		ret=vbound(zc_xtoi(tempstring1),-214748,214748)*10000;
		
		++ptr;
		char *ptr2=ptr;
		ptr2+=4;
		*ptr2=0;
		
		int32_t decval = abs(atoi(ptr));
		if(tempstring1[0] == '-')
		{
			if(ret == -2147480000)
				decval = vbound(decval, 0, 3648);
			ret-=decval;
		}
		else
		{
			if(ret == 2147480000)
				decval = vbound(decval, 0, 3647);
			ret+=decval;
		}
		
		return ret;
	}
	
	int32_t zc_chmod(const char* path, mode_t mode)
	{
#ifdef _WIN32
		return _chmod(path,mode);
#else
		return chmod(path,mode);
#endif
	}

	bool checkPath(const char* path, const bool is_dir)
	{
		struct stat info;

		if(stat( path, &info ) != 0)
			return false;
		else
		{
			return is_dir ? (info.st_mode & S_IFDIR)!=0 : (info.st_mode & S_IFDIR)==0;
		}
	}

	void safe_al_trace(const char* str)
	{
		size_t len = strlen(str);
		if(len < 512) //safe already
		{
			al_trace("%s",str);
			return;
		}
		else //Would crash al_trace
		{
			char buf[512] = {0};
			size_t q = 0;
			while(len-q >= 512)
			{
				memcpy(buf, str+q, 511);
				al_trace("%s",buf);
				q+=511;
			}
			if(len-q > 0)
			{
				al_trace("%s",str+q);
			}
		}
	}
	bool zc_isalpha(int c)
	{
	    if(unsigned(c) > 255) return false;
	    return isalpha((char)c);
	}
	
	string escape_string(char const* str)
	{
		ostringstream oss;
		oss << "\"";
		size_t len = strlen(str);
		for(size_t q = 0; q < len; ++q)
		{
			char c = str[q];
			if(c < ' ' || c > '~')
			{
				switch(c)
				{
					case '\n': oss << "\\n"; break;
					case '\t': oss << "\\t"; break;
					case '\a': oss << "\\a"; break;
					case '\b': oss << "\\b"; break;
					case '\f': oss << "\\f"; break;
					case '\r': oss << "\\r"; break;
					case '\v': oss << "\\v"; break;
					case '"': oss << "\\\""; break;
					case '\\': oss << "\\\\"; break;
					default:
					{
						oss << "\\x" << std::setfill('0') << std::setw(2) << std::hex << c;
						break;
					}
				}
			}
			else oss << c;
		}
		oss << "\"";
		return oss.str();
	}
	string escape_string(string const& str)
	{
		ostringstream oss;
		oss << "\"";
		size_t len = str.size();
		for(size_t q = 0; q < len; ++q)
		{
			char c = str[q];
			if(c < ' ' || c > '~')
			{
				switch(c)
				{
					case '\n': oss << "\\n"; break;
					case '\t': oss << "\\t"; break;
					case '\a': oss << "\\a"; break;
					case '\b': oss << "\\b"; break;
					case '\f': oss << "\\f"; break;
					case '\r': oss << "\\r"; break;
					case '\v': oss << "\\v"; break;
					case '"': oss << "\\\""; break;
					case '\\': oss << "\\\\"; break;
					default:
					{
						oss << "\\x" << std::setfill('0') << std::setw(2) << std::hex << (int32_t(c)&0xFF);
						break;
					}
				}
			}
			else oss << c;
		}
		oss << "\"";
		return oss.str();
	}
		
	string unescape_string(char const* str)
	{
		ostringstream oss;
		bool in_str = false;
		size_t len = strlen(str);
		for(size_t q = 0; q < len; ++q)
		{
			if(str[q] == '"')
			{
				if(in_str) break;
				else in_str = true;
			}
			else if(in_str)
			{
				if(str[q] == '\\')
				{
					if(++q < len)
					{
						switch(str[q])
						{
							case 'n': oss << '\n'; break;
							case 't': oss << '\t'; break;
							case 'a': oss << '\a'; break;
							case 'b': oss << '\b'; break;
							case 'f': oss << '\f'; break;
							case 'r': oss << '\r'; break;
							case 'v': oss << '\v'; break;
							case '\\': oss << '\\'; break;
							case '"': oss << '"'; break;
							case 'x':
							{
								if(q+2 < len)
								{
									char hb[3] = {str[q+1],str[q+2],0};
									oss << char(zc_xtoi(hb));
								}
								q += 2;
								break;
							}
							default: oss << str[q]; break;
						}
					}
				}
				else oss << str[q];
			}
		}
		return oss.str();
	}
	string unescape_string(string const& str)
	{
		ostringstream oss;
		bool in_str = false;
		size_t len = str.size();
		for(size_t q = 0; q < len; ++q)
		{
			if(str[q] == '"')
			{
				if(in_str) break;
				else in_str = true;
			}
			else if(in_str)
			{
				if(str[q] == '\\')
				{
					if(++q < len)
					{
						switch(str[q])
						{
							case 'n': oss << '\n'; break;
							case 't': oss << '\t'; break;
							case 'a': oss << '\a'; break;
							case 'b': oss << '\b'; break;
							case 'f': oss << '\f'; break;
							case 'r': oss << '\r'; break;
							case 'v': oss << '\v'; break;
							case '\\': oss << '\\'; break;
							case '"': oss << '"'; break;
							case 'x':
							{
								if(q+2 < len)
								{
									char hb[3] = {str[q+1],str[q+2],0};
									oss << char(zc_xtoi(hb));
								}
								q += 2;
								break;
							}
							default: oss << str[q]; break;
						}
					}
				}
				else oss << str[q];
			}
		}
		return oss.str();
	}
	size_t escstr_size(char const* str)
	{
		bool in_str = false;
		size_t len = strlen(str);
		size_t start = 0, q;
		bool complete = false;
		for(q = 0; q < len; ++q)
		{
			if(str[q] == '"')
			{
				if(in_str)
				{
					complete = true;
					break;
				}
				else
				{
					start = q;
					in_str = true;
				}
			}
			else if(in_str)
			{
				if(str[q] == '\\')
				{
					if(++q < len)
					{
						if(str[q] == 'x')
						{
							q += 2;
						}
					}
				}
			}
		}
		if(!complete) return 0;
		if(!in_str) return 0;
		return q-start+1;
	}
	
	string stringify_vector(vector<int32_t> const& vec, bool dec)
	{
		ostringstream oss;
		oss << "{";
		for(size_t q = 0; q < vec.size(); ++q)
		{
			if(dec)
			{
				int32_t val = vec[q];
				if(val % 10000)
				{
					char buf[16] = {0};
					sprintf(buf, "%d.%04d", val/10000, abs(val%10000));
					for(auto q = strlen(buf)-1; buf[q] == '0'; --q)
					{
						//Kill trailing zeroes
						buf[q] = 0;
					}
					oss << buf;
				}
				else
				{
					oss << std::to_string(vec[q]/10000);
				}
			}
			else oss << std::to_string(vec[q]);
			if(q+1 < vec.size())
				oss << ",";
		}
		oss << "}";
		return oss.str();
	}
	void unstringify_vector(vector<int32_t>& vec, string const& str, bool dec)
	{
		vec.clear();
		bool in_str = false;
		size_t len = str.size();
		for(size_t q = 0; q < len; ++q)
		{
			if(str[q] == '{')
				in_str = true;
			else if(str[q] == '}' && in_str)
				break;
			else if(in_str)
			{
				char buf[16] = {0};
				size_t ind = 0;
				while(str[q] == ',' && q+1 < len) ++q;
				for(char c = str[q]; c != ',' && c != '}' && q+1 < len; c = str[++q])
				{
					buf[ind++] = c;
				}
				if(buf[0])
				{
					if(dec)
						vec.push_back(ffparse2(buf));
					else vec.push_back(atoi(buf));
				}
				if(str[q] == '}') break;
			}
		}
	}
	void unstringify_vector(vector<int32_t>& vec, char const* str, bool dec)
	{
		vec.clear();
		bool in_str = false;
		size_t len = strlen(str);
		for(size_t q = 0; q < len; ++q)
		{
			if(str[q] == '{')
				in_str = true;
			else if(str[q] == '}' && in_str)
				break;
			else if(in_str)
			{
				char buf[16] = {0};
				size_t ind = 0;
				while(str[q] == ',' && q+1 < len) ++q;
				for(char c = str[q]; c != ',' && c != '}' && q+1 < len; c = str[++q])
				{
					buf[ind++] = c;
				}
				if(buf[0])
				{
					if(dec)
						vec.push_back(ffparse2(buf));
					else vec.push_back(atoi(buf));
				}
				if(str[q] == '}') break;
			}
		}
	}
	size_t vecstr_size(char const* str)
	{
		bool in_str = false;
		size_t len = strlen(str);
		size_t start = 0, q;
		bool complete = false;
		for(q = 0; q < len; ++q)
		{
			if(!in_str && str[q] == '{')
			{
				in_str = true;
				start = q;
			}
			else if(in_str)
			{
				if(str[q] == '}')
				{
					complete = true;
					break;
				}
				size_t ind = 0;
				while(str[q] == ',' && q+1 < len) ++q;
				for(char c = str[q]; c != ',' && c != '}' && q+1 < len; c = str[++q])
				{}
				if(str[q] == '}')
				{
					complete = true;
					break;
				}
			}
		}
		if(!complete) return 0;
		if(!in_str) return 0;
		return q-start+1;
	}

	std::filesystem::path create_new_file_path(std::filesystem::path dir, std::string filename_prefix, std::string ext)
	{
		auto path_prefix = dir / filename_prefix;
		std::filesystem::path new_path;
		int i = 1;
		do {
			new_path = fmt::format("{}-{:03}.{}", path_prefix.string(), i, ext);
			i += 1;
		} while (std::filesystem::exists(new_path));
		return new_path;
	}
}

using namespace util;
int32_t vbound(int32_t val, int32_t low, int32_t high)
{
	ASSERT(low <= high);
	if(val <= low)
		return low;
	if(val >= high)
		return high;
	return val;
}

double vbound(double val, double low, double high)
{
	ASSERT(low <= high);
	if(val <= low)
		return low;
	if(val >= high)
		return high;
	return val;
}

std::string dayextension(int32_t dy)
{
	char temp[6]; 
	switch(dy)
	{
		//st
		case 1:
		case 21:
		case 31:
			sprintf(temp,"%d%s",dy,"st"); 
			break;
		//nd
		case 2:
		case 22:
			sprintf(temp,"%d%s",dy,"nd"); 
			break;
		//rd
		case 3:
		case 23:
			sprintf(temp,"%d%s",dy,"rd"); 
			break;
		//th
		default:
			sprintf(temp,"%d%s",dy,"th");
			break;
	}
	
	return std::string(temp); 
}

bool fileexists(const char *filename)
{
	std::ifstream ifile(filename);
	if(ifile) return true;
	return false;
}

int32_t compare(int32_t a, int32_t b)
{
	if(a > b) return 1;
	if(a < b) return -1;
	return 0;
}

char const* get_snap_str()
{
	static char snap[16] = "snapshots/";
	static char nil[1] = {0};
	if(snap[9]==WRONG_PATH_SLASH) snap[9] = PATH_SLASH;
	if(checkPath("snapshots",true))
		return snap;
	do_mkdir("snapshots",PATH_MODE);
	if(checkPath("snapshots",true))
		return snap;
	return nil;
}

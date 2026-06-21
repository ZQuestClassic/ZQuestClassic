#include "base/containers.h"

#include <sstream>
using std::ostringstream;

void to_json(json& j, const bitstring& b)
{
	ostringstream oss;
	for (byte by : b.cont.inner())
	{
		for (byte b : {by & 0xF, (by >> 4) & 0xF})
		{
			if (b < 0xA)
				oss << char('0' + b);
			else
				oss << char('A' + b - 0xA);
		}
	}
	j = oss.str();
}
void from_json(const json& j, bitstring& b)
{
	string s;
	j.get_to(s);
	size_t ind = 0;
	auto len = s.size();
	auto& inner_c = b.cont.mut_inner();
	inner_c.clear();
	inner_c.reserve((len + 1) / 2);
	while (ind < len)
	{
		byte b = 0;
		
		for (int offs = 0; offs <= 4 && ind < len; offs += 4)
		{
			byte tmp = 0;
			char c = s[ind++];
			if (isdigit(c))
				tmp = c - '0';
			else if (c >= 'A' && c <= 'F')
				tmp = c - 'A';
			else if (c >= 'a' && c <= 'f')
				tmp = c - 'a';
			else throw std::invalid_argument("bitstring must contain only valid hex characters!");
			b |= tmp << offs;
		}
		inner_c.push_back(b);
	}
}


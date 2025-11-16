#include "zc/scripting//string_utils.h"

#include "base/ints.h"
#include "base/util.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/zelda.h"

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;
extern std::vector<int32_t> zs_vargs;

namespace {

bool is_valid_format(char c)
{
	switch(c)
	{
		case 'f': case 'd': case 'i': case 'p':
		case 'l': case 's': case 'c': case 'X':
		case 'x': case 'b': case 'B': case 'a':
			return true;
	}
	return false;
}

int32_t zspr_varg_getter(int32_t,int32_t next_arg)
{
	return zs_vargs.at(next_arg);
}

int32_t zspr_stack_getter(int32_t num_args, int32_t next_arg)
{
	return SH::read_stack(((ri->sp + num_args) - 1) - next_arg);
}

} // namespace

#define FORMATTER_FLAG_0FILL    0x01

const char* zs_formatter(const char* format, int32_t arg, int32_t mindig, dword flags)
{
	static std::string ret;
	
	ret.clear();
	if(format)
	{
		std::string mdstr = fmt::format("%{}{}{}",(flags&FORMATTER_FLAG_0FILL)?"0":"",
			mindig, (format[0] == 'x' || format[0] == 'X') ? format[0] : 'd');
		char const* mindigbuf = mdstr.c_str();
		bool tempbool = false;
		switch(format[0])
		{
			case 'f':
				tempbool = true;
				[[fallthrough]];
			case 'd':
				if(arg%10000)
					tempbool = true;
				[[fallthrough]];
			case 'i':
			case 'p':
			{
				char argbuf[32] = {0};
				bool neg = arg < 0;
				if(mindig)
					sprintf(argbuf,mindigbuf,arg / 10000);
				else util::zc_itoa(arg / 10000, argbuf);
				
				if(tempbool) //add decimal places
				{
					arg = abs(arg);
					auto ind = strlen(argbuf);
					argbuf[ind++] = '.';
					for(int div = 1000; div > 0; div /= 10)
						argbuf[ind++] = '0' + (arg/div)%10;
					for(--ind; argbuf[ind]=='0' && argbuf[ind-1]!='-'; --ind)
					{
						argbuf[ind] = 0;
					}
				}
				
				if(neg && argbuf[0] != '-')
					ret = "-";
				ret += argbuf;
				return ret.c_str();
			}
			//
			case 'l':
			{
				char argbuf[32] = {0};
				if(mindig)
					sprintf(argbuf, mindigbuf, arg);
				else util::zc_itoa(arg, argbuf);
				
				ret = argbuf;
				return ret.c_str();
			}
			//
			case 's':
			{
				if(mindig)
					Z_scripterrlog("Cannot use minimum digits flag for '%%s'\n");
				if(arg)
				{
					int32_t strptr = arg;
					ArrayManager am(strptr);
					if(am.invalid())
						ret = "<INVALID STRING>";
					else ArrayH::getString(strptr, ret, MAX_ZC_ARRAY_SIZE);
				}
				else ret = "<NULL>";
				return ret.c_str();
			}
			case 'c':
			{
				if(mindig)
					Z_scripterrlog("Cannot use minimum digits flag for '%%c'\n");
				int32_t c = (arg / 10000);
				if ( (byte(c)) != c )
				{
					Z_scripterrlog("Illegal char value (%d) passed to sprintf as '%%c' arg\n", c);
					Z_scripterrlog("Value of invalid char will overflow.\n");
				}
				ret.push_back(byte(c));
				return ret.c_str();
			}
			//
			case 'X':
				tempbool = true;
				[[fallthrough]];
			case 'x':
			{
				char argbuf[32] = {0};
				if(mindig)
					sprintf(argbuf,mindigbuf,arg / 10000);
				else util::zc_itoa( (arg/10000), argbuf, 16 ); //base 16; hex
				
				for ( int32_t inx = 0; inx < 16; ++inx ) //set chosen caps
				{
					argbuf[inx] = ( tempbool ? toupper(argbuf[inx]) : tolower(argbuf[inx]) );
				}
				ret = "0x";
				ret += argbuf;
				return ret.c_str();
			}
			//
			case 'b': //int binary
				arg /= 10000;
				[[fallthrough]];
			case 'B': //long binary
			{
				char argbuf[33] = {0};
				int num_digits = mindig;
				for(int q = num_digits; q < 32; ++q)
					if(arg&(1<<q))
						num_digits = q+1;
				for(int q = 0; q < num_digits; ++q)
				{
					argbuf[q] = (arg&(1<<(num_digits-q-1)))
						? '1' : '0';
				}
				ret = argbuf;
				return ret.c_str();
			}
			case 'a': //array
			{
				if(arg)
				{
					if(!is_valid_format(format[1]))
					{
						Z_scripterrlog("Format '%%a%c' is invalid!\n",format[1]);
						break;
					}
					ArrayManager am(arg);
					ret = am.asString([&](int32_t val)
						{
							return zs_formatter(format+1, val, mindig, flags);
						}, 214748);
				}
				else ret = "{ NULL }";
				return ret.c_str();
			}
			default:
			{
				Z_scripterrlog("Error: '%%%c' is not a valid printf argument.\n",format[0]);
				return ret.c_str();
			}
		}
	}
	Z_scripterrlog("Error: No format parameter given for zs_formatter\n");
	return ret.c_str();
}

std::string zs_sprintf(const char* format, int32_t num_args, std::function<int32_t(int32_t,int32_t)> arg_getter)
{
	int32_t next_arg = 0;
	bool is_old_args = get_qr(qr_OLD_PRINTF_ARGS);
	std::ostringstream oss;
	while(format[0] != '\0')
	{
		int32_t arg_val = 0;
		if(next_arg < num_args)
		{
			arg_val = arg_getter(num_args,next_arg);
		}
		else if(get_qr(qr_PRINTF_NO_0FILL))
		{
			oss << format;
			return oss.str();
		}
		char buf[256] = {0};
		for ( int32_t q = 0; q < 256; ++q )
		{
			if(format[0] == '\0') //done
			{
				oss << buf;
				return oss.str();
			}
			else if(format[0] == '%')
			{
				++format;
				int32_t min_digits = 0;
				dword formatter_flags = 0;
				if(format[0] >= '0' && format[0] <= '9' && !is_old_args)
				{
					char argbuf[4] = {0};
					int32_t q = 0;
					if(format[0] == '0') //Leading 0 means to 0-fill, and gets eaten
						formatter_flags |= FORMATTER_FLAG_0FILL;
					else --format; //else don't eat
					while(q < 4)
					{
						++format;
						char c = format[0];
						if(c == '\0')
						{
							Z_scripterrlog("Cannot use minimum digits flag with no argument\n");
							oss << buf;
							return oss.str();
						}
						if(c >= '0' && c <= '9')
							argbuf[q++] = c;
						else
						{
							--format;
							break;
						}
					}
					++format;
					min_digits = atoi(argbuf);
					if(!min_digits)
					{
						Z_scripterrlog("Error formatting string: Invalid number '%s'\n", argbuf);
					}
				}
				bool bin = (format[0] == 'b' || format[0] == 'B');
				bool hex = (format[0] == 'x' || format[0] == 'X');
				if(bin)
				{
					if(min_digits > 32)
					{
						Z_scripterrlog("Min digits argument cannot be larger than 32!"
							" Value will be truncated to 32.");
						min_digits = 32;
					}
				}
				else if(min_digits > 10)
				{
					Z_scripterrlog("Min digits argument cannot be larger than 10!"
						" Value will be truncated to 10.");
					min_digits = 10;
				}
				
				bool tempbool = false;
				switch( format[0] )
				{
					case 'd':
					case 'f':
					case 'i': case 'p':
					case 'l':
					case 's':
					case 'c':
					case 'x': case 'X':
					case 'b':  case 'B':
					{
						++next_arg;
						oss << buf << zs_formatter(format,arg_val,min_digits,formatter_flags);
						q = 300; //break main loop
						break;
					}
					case 'a': //array print
					{
						++next_arg;
						oss << buf << zs_formatter(format,arg_val,min_digits,formatter_flags);
						while(format[0] == 'a')
						{
							if(is_valid_format(format[1]))
								++format;
							else break;
						}
						q = 300; //break main loop
						break;
					}
					case '%':
					{
						if(min_digits)
							Z_scripterrlog("Cannot use minimum digits flag for '%%%%'\n");
						buf[q] = '%';
						break;
					}
					default:
					{
						if(is_old_args)
							buf[q] = format[0];
						else
						{
							Z_scripterrlog("Error: '%%%c' is not a valid printf argument.\n",format[0]);
						}
						break;
					}
				}
				++format;
			}
			else
			{
				buf[q] = format[0];
				++format;
			}
			if(q == 255)
			{
				oss << buf;
				break;
			}
		}
	}
	return oss.str();
}

void do_sprintf(const bool v, const bool varg)
{
	int32_t num_args, dest_arrayptr, format_arrayptr;
	if(varg)
	{
		num_args = zs_vargs.size();
		dest_arrayptr = SH::read_stack(ri->sp + 1);
		format_arrayptr = SH::read_stack(ri->sp);
	}
	else
	{
		num_args = SH::get_arg(sarg1, v) / 10000;
		dest_arrayptr = SH::read_stack(ri->sp + num_args + 1);
		format_arrayptr = SH::read_stack(ri->sp + num_args);
	}
	ArrayManager fmt_am(format_arrayptr);
	ArrayManager dst_am(dest_arrayptr);
	if(fmt_am.invalid() || dst_am.invalid())
		SET_D(rEXP1, 0);
	else
	{
		string formatstr;
		ArrayH::getString(format_arrayptr, formatstr, MAX_ZC_ARRAY_SIZE);
		
		string output = zs_sprintf(formatstr.c_str(), num_args, varg ? zspr_varg_getter : zspr_stack_getter);
		if(ArrayH::setArray(dest_arrayptr, output, true) == SH::_Overflow)
		{
			Z_scripterrlog("Dest string supplied to 'sprintf()' not large enough and cannot be resized\n");
			SET_D(rEXP1, ArrayH::strlen(dest_arrayptr));
		}
		else SET_D(rEXP1, output.size());
	}
	if(varg)
		zs_vargs.clear();
}

void do_sprintfarr()
{
	int32_t dest_arrayptr = SH::read_stack(ri->sp + 2),
		format_arrayptr = SH::read_stack(ri->sp + 1),
		args_arrayptr = SH::read_stack(ri->sp + 0);
	ArrayManager fmt_am(format_arrayptr);
	ArrayManager arg_am(args_arrayptr);
	ArrayManager dst_am(dest_arrayptr);
	if(fmt_am.invalid() || arg_am.invalid() || dst_am.invalid())
		SET_D(rEXP1, 0);
	else
	{
		auto num_args = arg_am.size();
		string formatstr;
		ArrayH::getString(format_arrayptr, formatstr, MAX_ZC_ARRAY_SIZE);
		
		string output = zs_sprintf(formatstr.c_str(), num_args,
			[&](int32_t,int32_t next_arg)
			{
				return arg_am.get(next_arg);
			});
		
		if(ArrayH::setArray(dest_arrayptr, output, true) == SH::_Overflow)
		{
			Z_scripterrlog("Dest string supplied to 'sprintfa()' not large enough and cannot be resized\n");
			SET_D(rEXP1, ArrayH::strlen(dest_arrayptr));
		}
		else SET_D(rEXP1, output.size());
	}
}

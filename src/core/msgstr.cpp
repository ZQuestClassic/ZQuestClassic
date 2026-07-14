#include "msgstr.h"
#include "allegro/debug.h"
#include "components/scc/scc.h"
#include <string>

MsgStr *MsgStrings;

void message_portrait::clear()
{
	*this = message_portrait();
}

// Copy everything except listpos
MsgStr& MsgStr::operator=(MsgStr const& other)
{
	copyText(other);
	copyStyle(other);
	return *this;
}

// Copy text data - just s and nextstring
void MsgStr::copyText(MsgStr const& other)
{
	encoding_type=other.encoding_type;
	s.resize(other.s.size());
	s.assign(other.s.c_str());
	nextstring=other.nextstring;
	parse(); // TODO: this is definitely double-parsing on qst load.
}

// Copy style data - everything except s, nextstring, and listpos
void MsgStr::copyStyle(MsgStr const& other)
{
	tile=other.tile;
	cset=other.cset;
	trans=other.trans;
	font=other.font;
	x=other.x;
	y=other.y;
	w=other.w;
	h=other.h;
	sfx = other.sfx;
	adv_sfx = other.adv_sfx;
	menu_move_sfx = other.menu_move_sfx;
	menu_close_sfx = other.menu_close_sfx;
	vspace=other.vspace;
	hspace=other.hspace;
	stringflags=other.stringflags;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = other.margins[q];
	}
	portrait = other.portrait;
	shadow_type=other.shadow_type;
	shadow_color=other.shadow_color;
	drawlayer=other.drawlayer;
	
	icon_more = other.icon_more;
	icon_scroll_up = other.icon_scroll_up;
	icon_scroll_down = other.icon_scroll_down;
	active_scroll_speed = other.active_scroll_speed;
	passive_scroll_speed = other.passive_scroll_speed;
}

void MsgStr::copyAll(MsgStr const& other)
{
	copyText(other);
	copyStyle(other);
	listpos = other.listpos;
}

void MsgStr::advpaste(MsgStr const& other, bitstring const& flags)
{
	if(flags.get(STR_ADVP_TEXT))
	{
		s = other.s;
		encoding_type = other.encoding_type;
		parsed_msg_str = {};
	}
	if(flags.get(STR_ADVP_NEXTSTR))
		nextstring = other.nextstring;
	if(flags.get(STR_ADVP_POSSZ))
	{
		x = other.x;
		y = other.y;
		w = other.w;
		h = other.h;
	}
	if(flags.get(STR_ADVP_PORTRAIT))
	{
		portrait = other.portrait;
	}
	if(flags.get(STR_ADVP_BACKGROUND))
	{
		tile = other.tile;
		cset = other.cset;
	}
	if(flags.get(STR_ADVP_FONT))
		font = other.font;
	if (flags.get(STR_ADVP_SFX))
	{
		sfx = other.sfx;
		adv_sfx = other.adv_sfx;
		menu_move_sfx = other.menu_move_sfx;
		menu_close_sfx = other.menu_close_sfx;
	}
	if(flags.get(STR_ADVP_SPACING))
	{
		hspace = other.hspace;
		vspace = other.vspace;
	}
	if(flags.get(STR_ADVP_MARGINS))
		for(int q = 0; q < 4; ++q)
			margins[q] = other.margins[q];
	if(flags.get(STR_ADVP_SHADOW))
	{
		shadow_type = other.shadow_type;
		shadow_color = other.shadow_color;
	}
	if(flags.get(STR_ADVP_LAYER))
		drawlayer = other.drawlayer;
	if(flags.get(STR_ADVP_FLAGS))
		stringflags = other.stringflags;
	
	if (flags.get(STR_ADVP_ICON_MORE))
		icon_more = other.icon_more;
	if (flags.get(STR_ADVP_ICON_SCROLL_UP))
		icon_scroll_up = other.icon_scroll_up;
	if (flags.get(STR_ADVP_ICON_SCROLL_DOWN))
		icon_scroll_down = other.icon_scroll_down;
	if (flags.get(STR_ADVP_SCROLL_SPEEDS))
	{
		active_scroll_speed = other.active_scroll_speed;
		passive_scroll_speed = other.passive_scroll_speed;
	}
	
}

void MsgStr::clear()
{
	s = "";
	s.shrink_to_fit();
	encoding_type = EncodingType::Binary;
	parsed_msg_str = {};
	segments_as_zfix_array.clear();
	nextstring = 0;
	tile=0;
	cset=0;
	trans=0;
	font=0;
	x=0;
	y=0;
	w=0;
	h=0;
	sfx = adv_sfx = menu_move_sfx = menu_close_sfx = 0;
	vspace=0;
	hspace=0;
	stringflags=0;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = 0;
	}
	portrait.clear();
	shadow_type=0;
	shadow_color=0;
	listpos=0;
	drawlayer=6;
	icon_more = message_icon();
	icon_scroll_up = message_icon();
	icon_scroll_down = message_icon();
	active_scroll_speed = 1;
	passive_scroll_speed = 1;
}

warnings MsgStr::setFromAsciiEncoding(std::string text)
{
	s = std::move(text);
	encoding_type = EncodingType::Ascii;
	return parse();
}

warnings MsgStr::setFromLegacyEncoding(std::string text)
{
	s = std::move(text);
	encoding_type = EncodingType::Binary;
	return parse();
}

warnings MsgStr::set(std::string text, EncodingType encoding_type)
{
	return encoding_type == EncodingType::Ascii ?
		setFromAsciiEncoding(std::move(text)) :
		setFromLegacyEncoding(std::move(text));
}

warnings MsgStr::parse() const
{
	auto [parsed_msg_str, warnings] = encoding_type == EncodingType::Ascii ? parse_ascii_msg_str(s) : parse_legacy_binary_msg_str(s);
	if (warnings.size())
	{
		al_trace("Warning: found message string with SCC warnings: %s\n", parsed_msg_str.serialize().c_str());
		for (auto& warning : warnings)
			al_trace("\t%s\n", warning.c_str());
	}

	this->parsed_msg_str = std::move(parsed_msg_str);
	segments_as_zfix_array.clear();
	return warnings;
}

void MsgStr::ensureLegacyEncoding()
{
	if (encoding_type == EncodingType::Binary)
		return;

	parse();

	auto [binary, warnings] = parsed_msg_str.serialize_legacy();
	if (warnings.size())
	{
		al_trace("Warning: lossy conversion of message string to the legacy encoding: %s\n", s.c_str());
		for (auto& warning : warnings)
			al_trace("\t%s\n", warning.c_str());
	}

	s = std::move(binary);
	parsed_msg_str = {};
	encoding_type = EncodingType::Binary;
}

void MsgStr::ensureAsciiEncoding()
{
	if (encoding_type == EncodingType::Ascii)
		return;

	s = serialize();
	parsed_msg_str = {};
	encoding_type = EncodingType::Ascii;
}

// Note: this always returns an ascii-compatible encoding.
std::string MsgStr::serialize() const
{
	if (parsed_msg_str.literals.empty() && parsed_msg_str.commands.empty())
		parse();

	return parsed_msg_str.serialize();
}

const std::vector<zfix>& MsgStr::segmentsAsZFixArray() const
{
	if (parsed_msg_str.literals.empty() && parsed_msg_str.commands.empty())
		parse();

	if (segments_as_zfix_array.empty())
	{
		size_t literals = 0;
		size_t commands = 0;
		for (int i = 0; i < parsed_msg_str.segment_types.size(); i++)
		{
			auto type = parsed_msg_str.segment_types[i];
			segments_as_zfix_array.push_back((int)type);

			if (type == ParsedMsgStr::SegmentType::Command)
			{
				auto& cmd = parsed_msg_str.commands[commands++];

				segments_as_zfix_array.push_back(cmd.start); // source start
				segments_as_zfix_array.push_back(cmd.length); // source length
				segments_as_zfix_array.push_back(cmd.code);
				segments_as_zfix_array.push_back(cmd.num_args);
				for (int j = 0; j < cmd.num_args; j++)
					segments_as_zfix_array.push_back(cmd.args[j]);
			}
			else
			{
				auto& literal = parsed_msg_str.literals[literals++];
				size_t offset = literal.data() - s.data();
				segments_as_zfix_array.push_back(int(offset)); // source start
				segments_as_zfix_array.push_back(int(literal.size())); // source length
			}
		}
	}

	return segments_as_zfix_array;
}

MsgStr::iterator MsgStr::create_iterator() const
{
	if (parsed_msg_str.literals.empty() && parsed_msg_str.commands.empty())
		parse();

	MsgStr::iterator it{this};
	return it;
}

bool MsgStr::iterator::next()
{
	return next_character();
}

void MsgStr::iterator::set_buffer(std::string text)
{
	buffer = std::move(text);
	state = buffer.empty() ? IDLE : CHARACTER;
	word = "";
	j = 0;
	k = 0;
}

bool MsgStr::iterator::done() const
{
	return state == DONE;
}

std::string MsgStr::iterator::peek(byte n) const
{
	if (k + n < word.length())
		return word.substr(k + n, 1);

	size_t i = j + n - (word.length() - k + 1);
	if (i < buffer.length())
		return buffer.substr(i, 1);

	return "";
}

const char* MsgStr::iterator::remaining_word() const
{
	return word.c_str() + k - 1;
}

bool MsgStr::iterator::next_segment()
{
	buffer = "";
	j = 0;
	k = 0;

	if (str->parsed_msg_str.segment_types.size() <= segment_index)
	{
		state = DONE;
		return true;
	}

	auto segment_type = str->parsed_msg_str.segment_types[segment_index++];
	if (segment_type == ParsedMsgStr::SegmentType::Command)
	{
		state = COMMAND;
		command = str->parsed_msg_str.commands[command_index++];
	}
	else
	{
		state = CHARACTER;
		buffer = str->parsed_msg_str.literals[literal_index++];
	}

	return false;
}

bool MsgStr::iterator::next_word()
{
	k = 0;
	word = "";

	if (j >= buffer.size())
	{
		if (post_segment_delay > 0)
		{
			state = IDLE;
			if (!post_segment_delay_forced && post_segment_delay_fast)
				post_segment_delay = zc_max(0, post_segment_delay - 5);
			else
				post_segment_delay--;
			return false;
		}

		if (next_segment()) return true;
	}

	// Buffer is empty here only when processing a command.
	if (buffer.empty())
		return false;

	// Set `text` to the "word" currently being processed.
	// If string wrapping is enabled, this is the entire word leading up to the next space (or end of the strings).
	// Note that `s` may only be from a string liteal, so commands in-between literals also
	// counts as a word boundrary for wrapping purposes.
	if (str->stringflags & STRINGFLAG_WRAP)
	{
		// Spaces are like one-character words.
		if (buffer[j] == ' ')
		{
			word = buffer[j++];
		}
		else
		{
			// Complete words finish at spaces.
			int k = j;
			while (j < buffer.size() && buffer[j] != ' ')
				j++;
			word = buffer.substr(k, j - k);
		}
	}
	else
	{
		word = buffer[j++];
	}

	return false;
}

bool MsgStr::iterator::next_character()
{
	character = "";

	if (state == 0 || k == word.size())
		if (next_word()) return true;

	if (state == COMMAND)
		return false;

	if (k < word.size())
	{
		state = CHARACTER;
		character = word[k++];
		return false;
	}

	state = IDLE;
	return false;
}

void MsgStr::iterator::set_post_segment_delay(int frames, bool forced)
{
	post_segment_delay = frames;
	post_segment_delay_forced = forced;
	post_segment_delay_fast = false;
}

void MsgStr::iterator::set_post_segment_delay_fast(bool fast)
{
	post_segment_delay_fast = fast;
}

int MsgStr::iterator::get_post_segment_delay() const
{
	return post_segment_delay;
}

bool MsgStr::iterator::get_post_segment_delay_forced() const
{
	return post_segment_delay_forced;
}

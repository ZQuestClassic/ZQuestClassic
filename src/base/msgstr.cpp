#include "msgstr.h"
#include "allegro/debug.h"

MsgStr *MsgStrings;

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
	s.resize(other.s.size());
	s.assign(other.s.c_str());
	nextstring=other.nextstring;
	parse();
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
	sfx=other.sfx;
	vspace=other.vspace;
	hspace=other.hspace;
	stringflags=other.stringflags;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = other.margins[q];
	}
	portrait_tile=other.portrait_tile;
	portrait_cset=other.portrait_cset;
	portrait_x=other.portrait_x;
	portrait_y=other.portrait_y;
	portrait_tw=other.portrait_tw;
	portrait_th=other.portrait_th;
	shadow_type=other.shadow_type;
	shadow_color=other.shadow_color;
	drawlayer=other.drawlayer;
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
		portrait_tile = other.portrait_tile;
		portrait_cset = other.portrait_cset;
		portrait_x = other.portrait_x;
		portrait_y = other.portrait_y;
		portrait_tw = other.portrait_tw;
		portrait_th = other.portrait_th;
	}
	if(flags.get(STR_ADVP_BACKGROUND))
	{
		tile = other.tile;
		cset = other.cset;
	}
	if(flags.get(STR_ADVP_FONT))
		font = other.font;
	if(flags.get(STR_ADVP_SFX))
		sfx = other.sfx;
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
}

void MsgStr::clear()
{
	s = "";
	s.shrink_to_fit();
	parsed_msg_str = {};
	nextstring = 0;
	tile=0;
	cset=0;
	trans=0;
	font=0;
	x=0;
	y=0;
	w=0;
	h=0;
	sfx=0;
	vspace=0;
	hspace=0;
	stringflags=0;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = 0;
	}
	portrait_tile=0;
	portrait_cset=0;
	portrait_x=0;
	portrait_y=0;
	portrait_tw=0;
	portrait_th=0;
	shadow_type=0;
	shadow_color=0;
	listpos=0;
	drawlayer=6;
}

void MsgStr::setFromLegacyEncoding(std::string text)
{
	s = text;
	parse();
}

void MsgStr::parse() const
{
	auto [parsed_msg_str, warnings] = parse_legacy_msg_str(s);
	if (warnings.size())
	{
		al_trace("Warning: found message string with SCC warnings: %s\n", parsed_msg_str.serialize().c_str());
		for (auto& error : warnings)
			al_trace("\t%s\n", error.c_str());
	}

	this->parsed_msg_str = std::move(parsed_msg_str);
}

std::string MsgStr::serialize() const
{
	if (parsed_msg_str.literals.empty())
		parse();
	return parsed_msg_str.serialize();
}

MsgStr::iterator MsgStr::create_iterator() const
{
	if (parsed_msg_str.literals.empty())
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
	buffer = text;
	state = text.empty() ? IDLE : CHARACTER;
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

	if (str->parsed_msg_str.segment_types[segment_index++] == 0)
	{
		state = CHARACTER;
		buffer = str->parsed_msg_str.literals[literal_index++];
	}
	else
	{
		state = COMMAND;
		command = str->parsed_msg_str.commands[command_index++];
	}

	return false;
}

bool MsgStr::iterator::next_word()
{
	k = 0;
	word = "";

	if (j >= buffer.size())
	{
		if (post_segment_delay)
		{
			state = IDLE;
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

#ifndef MSGSTR_H_
#define MSGSTR_H_

#include <string>
#include "base/ints.h"
#include "base/headers.h"
#include "base/containers.h"
#include "base/scc.h"

#define MSG_NEW_SIZE 8192
#define MSGBUF_SIZE (MSG_NEW_SIZE*8)

#define STRINGFLAG_WRAP             0x01
#define STRINGFLAG_CONT             0x02
#define STRINGFLAG_CENTER           0x04
#define STRINGFLAG_RIGHT            0x08
#define STRINGFLAG_FULLTILE         0x10
#define STRINGFLAG_TRANS_BG         0x20
#define STRINGFLAG_TRANS_FG         0x40
enum
{
	STR_ADVP_TEXT,
	STR_ADVP_NEXTSTR,
	STR_ADVP_POSSZ,
	STR_ADVP_PORTRAIT,
	STR_ADVP_BACKGROUND,
	STR_ADVP_FONT,
	STR_ADVP_SFX,
	STR_ADVP_SPACING,
	STR_ADVP_MARGINS,
	STR_ADVP_SHADOW,
	STR_ADVP_LAYER,
	STR_ADVP_FLAGS,
	STR_ADVP_SZ
};

struct MsgStr
{
	// This is the underlying, raw data. SCC commands are encoded in a special way.
	// Use `serialize` instead to get a human readable string.
	std::string s;
	mutable ParsedMsgStr parsed_msg_str;
	word nextstring;
	int32_t tile;
	byte cset;
	bool trans;
	byte font;
	int16_t x;
	int16_t y;
	uint16_t w;
	uint16_t h;
	byte sfx;
	word listpos;
	byte vspace;
	byte hspace;
	byte stringflags;
	int16_t margins[4];
	int32_t portrait_tile;
	byte portrait_cset;
	byte portrait_x;
	byte portrait_y;
	byte portrait_tw;
	byte portrait_th;
	byte shadow_type;
	byte shadow_color;
	byte drawlayer;
	
	// Copy everything except listpos
	MsgStr& operator=(MsgStr const& other);
	// Copy text data - just s and nextstring
	void copyText(MsgStr const& other);
	// Copy style data - everything except s, nextstring, and listpos
	void copyStyle(MsgStr const& other);
	void copyAll(MsgStr const& other);
	void advpaste(MsgStr const& other, bitstring const& pasteflags);
	void clear();
	void setFromLegacyEncoding(std::string text);
	void parse() const;
	// A human-readable encoding of the string.
	std::string serialize() const;

	struct iterator
	{
		enum state {
			NOT_STARTED,
			CHARACTER,
			COMMAND,
			IDLE,
			DONE,
		};

		iterator(const MsgStr* str) : str(str) {}

		// Do one iteration of processing the msg str, and sets `state` as appropriate.
		// Returns true if done.
		bool next();
		void set_buffer(std::string text);
		// Returns true if done.
		bool done() const;
		// Returns then nth next character (where 0 is the next one).
		// Only looks as far as the current buffer - the start of the next segment is a breakpoint.
		std::string peek(byte n) const;
		// Returns the rest of the current word.
		const char* remaining_word() const;

		state state = NOT_STARTED;
		// A single character. This is a string because eventually this may contain a unicode
		// character, which is larger than one byte.
		// This is only valid if `state` is CHARACTER.
		std::string character;
		// This is only valid if `state` is COMMAND.
		StringCommand command;
		// How many frames to idle after a segment finishes.
		int post_segment_delay = 0;

	private:
		bool next_segment();
		bool next_word();
		bool next_character();

		const MsgStr* str;
		// The text from the current segment.
		std::string buffer;
		// The text from buffer for the current "word".
		std::string word;
		// The next index to use for `str->parsed_msg_str.segment_types`.
		int segment_index = 0;
		// The next index to use for `str->parsed_msg_str.literals`.
		int literal_index = 0;
		// The next index to use for `str->parsed_msg_str.commands`.
		int command_index = 0;
		// The index into `buffer`, where the next `word` will begin.
		int j = 0;
		// The index into `word` - `character` will be set based on this when `next()` is called.
		int k = 0;
	};

	iterator create_iterator() const;
};

extern MsgStr* MsgStrings;

#endif


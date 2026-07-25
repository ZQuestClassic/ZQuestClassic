// Unit tests for MsgStr::iterator (src/core/msgstr.cpp).
//
// These cover just the iterator's state machine - the message renderer built on
// top of it (word-wrap measurement, margins, timing) lives in
// src/zc/message_string.cpp and is exercised by replay tests.

#include "core/msgstr.h"
#include "components/scc/scc.h"
#include "test_runner/test_runner.h"
#include "test_runner/assert.h"

#include <string>
#include <vector>

namespace
{

using IterState = MsgStr::iterator::State;

struct IterEvent
{
	IterState state;
	std::string character; // valid when state is CHARACTER
	int command_code = -1; // valid when state is COMMAND
};

// Initializes in place (rather than returning by value) because parsed_msg_str
// holds string_views into `s` - copying or moving a MsgStr would leave them
// dangling.
void init_msg(MsgStr& msg, std::string text, bool wrap = true,
	MsgStr::EncodingType encoding = MsgStr::EncodingType::Ascii, bool expect_warnings = false)
{
	msg.stringflags = wrap ? STRINGFLAG_WRAP : 0;
	auto warnings = msg.set(std::move(text), encoding);
	if (!expect_warnings)
		assertSize(warnings, 0);
}

IterEvent record_event(const MsgStr::iterator& it)
{
	IterEvent event{};
	event.state = it.state;
	if (it.state == MsgStr::iterator::CHARACTER)
		event.character = it.character;
	else if (it.state == MsgStr::iterator::COMMAND)
		event.command_code = it.command.code;
	return event;
}

// Repeatedly calls next() until the iterator reports done, recording each
// resulting state. Commands are stepped over without injecting replacement
// text (see test_set_buffer_* for that).
std::vector<IterEvent> drain(MsgStr::iterator& it)
{
	std::vector<IterEvent> events;
	while (!it.next())
	{
		events.push_back(record_event(it));
		assertTrue(events.size() < 200); // runaway guard
	}
	return events;
}

std::string characters_of(const std::vector<IterEvent>& events)
{
	std::string result;
	for (auto& event : events)
	{
		assertEqual(event.state, MsgStr::iterator::CHARACTER);
		result += event.character;
	}
	return result;
}

// Regression test for blank Guy strings softlocking the game (8293f57479):
// a string with no segments must report done on the first advance - and not
// before, since NOT_STARTED is how tick_message knows to begin at all.
void test_empty_string_done_on_first_advance()
{
	MsgStr msg{};
	init_msg(msg, "");
	auto it = msg.create_iterator();

	assertEqual(it.state, MsgStr::iterator::NOT_STARTED);
	assertTrue(!it.done());

	assertTrue(it.next());
	assertTrue(it.done());
	assertEqual(it.state, MsgStr::iterator::DONE);
}

void test_next_after_done_stays_done()
{
	MsgStr msg{};
	init_msg(msg, "");
	auto it = msg.create_iterator();

	assertTrue(it.next());
	assertTrue(it.next());
	assertTrue(it.done());
}

// The other half of the blank-string bug: 1.90-era "empty" strings are
// actually all spaces, which iterate as characters and only then hit DONE.
void test_spaces_only_string()
{
	MsgStr msg{};
	init_msg(msg, "   ");
	auto it = msg.create_iterator();

	auto events = drain(it);
	assertEqual(characters_of(events), std::string("   "));
	assertTrue(it.done());
}

// The legacy binary parser trims trailing spaces, so a 1.90-era space-padded
// "empty" string parses to zero segments and behaves exactly like the empty
// string above. (This is the data shape that triggered the softlock.)
void test_spaces_only_string_legacy_encoding()
{
	MsgStr msg{};
	init_msg(msg, "   ", true, MsgStr::EncodingType::Binary);
	auto it = msg.create_iterator();

	assertTrue(!it.done());
	assertTrue(it.next());
	assertTrue(it.done());
}

void test_literal_characters_in_order()
{
	MsgStr msg{};
	init_msg(msg, "Hi!");
	auto it = msg.create_iterator();

	auto events = drain(it);
	assertEqual(characters_of(events), std::string("Hi!"));
	assertTrue(it.done());
}

// With STRINGFLAG_WRAP, characters are grouped into words (spaces are
// one-character words), and remaining_word() reports the current character
// plus the rest of its word - this is the contract the renderer's wrap
// measurement depends on.
void test_wrap_word_grouping()
{
	MsgStr msg{};
	init_msg(msg, "ab cd", true);
	auto it = msg.create_iterator();

	it.next();
	assertEqual(it.character, std::string("a"));
	assertEqual(std::string(it.remaining_word()), std::string("ab"));

	it.next();
	assertEqual(it.character, std::string("b"));
	assertEqual(std::string(it.remaining_word()), std::string("b"));

	it.next();
	assertEqual(it.character, std::string(" "));
	assertEqual(std::string(it.remaining_word()), std::string(" "));

	it.next();
	assertEqual(it.character, std::string("c"));
	assertEqual(std::string(it.remaining_word()), std::string("cd"));

	it.next();
	assertEqual(it.character, std::string("d"));
	assertEqual(std::string(it.remaining_word()), std::string("d"));

	assertTrue(it.next());
}

// Without STRINGFLAG_WRAP, every character is its own word.
void test_no_wrap_single_char_words()
{
	MsgStr msg{};
	init_msg(msg, "ab", false);
	auto it = msg.create_iterator();

	it.next();
	assertEqual(it.character, std::string("a"));
	assertEqual(std::string(it.remaining_word()), std::string("a"));

	it.next();
	assertEqual(it.character, std::string("b"));
	assertEqual(std::string(it.remaining_word()), std::string("b"));

	assertTrue(it.next());
}

// A string that is only a command: the consumer sees COMMAND once, and the
// following advance reports done. (Commands as the final segment have been a
// recurring source of end-of-string bugs; see 118e26fe6a.)
void test_command_only_string()
{
	MsgStr msg{};
	init_msg(msg, R"(\Speed\5\)");
	auto it = msg.create_iterator();

	assertTrue(!it.next());
	assertEqual(it.state, MsgStr::iterator::COMMAND);
	assertEqual((int)it.command.code, MSGC_SPEED);
	assertEqual((int)it.command.num_args, 1);
	assertEqual(it.command.args[0].getInt(), 5);

	assertTrue(it.next());
	assertTrue(it.done());
}

void test_command_as_final_segment()
{
	MsgStr msg{};
	init_msg(msg, R"(Hi \Speed\5\)");
	auto it = msg.create_iterator();

	it.next();
	assertEqual(it.character, std::string("H"));
	it.next();
	assertEqual(it.character, std::string("i"));
	it.next();
	assertEqual(it.character, std::string(" "));

	it.next();
	assertEqual(it.state, MsgStr::iterator::COMMAND);
	assertEqual((int)it.command.code, MSGC_SPEED);

	assertTrue(it.next());
	assertTrue(it.done());
}

void test_command_between_literals()
{
	MsgStr msg{};
	init_msg(msg, R"(ab\Speed\1\ cd)");
	auto it = msg.create_iterator();

	auto events = drain(it);
	assertSize(events, 5);
	assertEqual(events[0].character, std::string("a"));
	assertEqual(events[1].character, std::string("b"));
	assertEqual(events[2].state, MsgStr::iterator::COMMAND);
	assertEqual(events[2].command_code, MSGC_SPEED);
	assertEqual(events[3].character, std::string("c"));
	assertEqual(events[4].character, std::string("d"));
}

void test_consecutive_commands()
{
	MsgStr msg{};
	init_msg(msg, R"(\Speed\1\ \Speed\2\)");
	auto it = msg.create_iterator();

	it.next();
	assertEqual(it.state, MsgStr::iterator::COMMAND);
	assertEqual(it.command.args[0].getInt(), 1);

	it.next();
	assertEqual(it.state, MsgStr::iterator::COMMAND);
	assertEqual(it.command.args[0].getInt(), 2);

	assertTrue(it.next());
}

void test_newline_is_a_command()
{
	MsgStr msg{};
	init_msg(msg, "Hi\nyo");
	auto it = msg.create_iterator();

	auto events = drain(it);
	assertSize(events, 5);
	assertEqual(events[1].character, std::string("i"));
	assertEqual(events[2].state, MsgStr::iterator::COMMAND);
	assertEqual(events[2].command_code, MSGC_NEWLINE);
	assertEqual(events[3].character, std::string("y"));
}

// Invalid commands roundtrip as literals, so the iterator plays their raw
// source text back as characters.
void test_invalid_command_iterates_as_text()
{
	std::string text = R"(\18\1\2\3\4\5\6)"; // missing trailing slash
	MsgStr msg{};
	init_msg(msg, text, true, MsgStr::EncodingType::Ascii, /*expect_warnings*/ true);
	auto it = msg.create_iterator();

	auto events = drain(it);
	assertEqual(characters_of(events), text);
}

// Commands that produce text (like \Name\) have the consumer inject a
// replacement buffer; the injected text plays out before the next segment.
void test_set_buffer_injects_text()
{
	MsgStr msg{};
	init_msg(msg, R"(X\Name\ Y)");
	auto it = msg.create_iterator();

	it.next();
	assertEqual(it.character, std::string("X"));

	it.next();
	assertEqual(it.state, MsgStr::iterator::COMMAND);
	assertEqual((int)it.command.code, MSGC_NAME);

	it.set_buffer("Zelda");
	assertEqual(it.state, MsgStr::iterator::CHARACTER);

	auto events = drain(it);
	assertEqual(characters_of(events), std::string("ZeldaY"));
}

// Injecting an empty buffer marks the command processed (IDLE); the next
// advance moves on to the following segment.
void test_set_buffer_empty_steps_past_command()
{
	MsgStr msg{};
	init_msg(msg, R"(X\Speed\1\ Y)");
	auto it = msg.create_iterator();

	it.next();
	it.next();
	assertEqual(it.state, MsgStr::iterator::COMMAND);

	it.set_buffer("");
	assertEqual(it.state, MsgStr::iterator::IDLE);

	it.next();
	assertEqual(it.state, MsgStr::iterator::CHARACTER);
	assertEqual(it.character, std::string("Y"));

	assertTrue(it.next());
}

// A post-segment delay makes the iterator report IDLE (once per advance) after
// the current buffer runs out, before moving to the next segment.
void test_post_segment_delay()
{
	MsgStr msg{};
	init_msg(msg, "AB");
	auto it = msg.create_iterator();

	it.next();
	it.next();
	assertEqual(it.character, std::string("B"));

	it.set_post_segment_delay(2, false);
	assertEqual(it.get_post_segment_delay(), 2);

	assertTrue(!it.next());
	assertEqual(it.state, MsgStr::iterator::IDLE);
	assertEqual(it.get_post_segment_delay(), 1);

	assertTrue(!it.next());
	assertEqual(it.state, MsgStr::iterator::IDLE);
	assertEqual(it.get_post_segment_delay(), 0);

	assertTrue(it.next());
	assertTrue(it.done());
}

// Holding A ("fast") burns through an unforced delay 5 frames per advance.
void test_post_segment_delay_fast()
{
	MsgStr msg{};
	init_msg(msg, "A");
	auto it = msg.create_iterator();

	it.next();
	it.set_post_segment_delay(7, false);
	it.set_post_segment_delay_fast(true);

	assertTrue(!it.next());
	assertEqual(it.state, MsgStr::iterator::IDLE);
	assertEqual(it.get_post_segment_delay(), 2);

	assertTrue(!it.next());
	assertEqual(it.get_post_segment_delay(), 0);

	assertTrue(it.next());
}

// A forced delay ignores the fast flag.
void test_post_segment_delay_forced()
{
	MsgStr msg{};
	init_msg(msg, "A");
	auto it = msg.create_iterator();

	it.next();
	it.set_post_segment_delay(2, true);
	it.set_post_segment_delay_fast(true);
	assertTrue(it.get_post_segment_delay_forced());

	assertTrue(!it.next());
	assertEqual(it.get_post_segment_delay(), 1);

	assertTrue(!it.next());
	assertEqual(it.get_post_segment_delay(), 0);

	assertTrue(it.next());
}

void test_peek_within_word()
{
	MsgStr msg{};
	init_msg(msg, "abc");
	auto it = msg.create_iterator();

	// Before the first advance there is nothing to peek at.
	assertEqual(it.peek(0), std::string(""));

	it.next();
	assertEqual(it.character, std::string("a"));
	assertEqual(it.peek(0), std::string("b"));
	assertEqual(it.peek(1), std::string("c"));
	assertEqual(it.peek(3), std::string(""));
}

void test_peek_at_word_boundary()
{
	MsgStr msg{};
	init_msg(msg, "ab cd");
	auto it = msg.create_iterator();

	it.next(); // 'a', current word is "ab"
	assertEqual(it.peek(0), std::string("b"));

	// NOTE: this pins current behavior, which is off by one at the boundary
	// between the current word and the rest of the buffer: peeks past the end
	// of the current word repeat the word's last character instead of reading
	// the character that follows it (peek(1) here "should" be " "). The only
	// consumer peeking past the current character is the double-space check in
	// tick_message, so a fix would need a replay version gate.
	assertEqual(it.peek(1), std::string("b"));

	it.next(); // 'b', word exhausted
	assertEqual(it.peek(0), std::string("b"));
	assertEqual(it.peek(1), std::string(" "));
	assertEqual(it.peek(2), std::string("c"));
}

// serialize(type) re-encodes without modifying the string. Game->GetMessage
// relies on this to not permanently convert (and, for the legacy encoding,
// lossily downgrade) a string as a side effect of reading it.
void test_serialize_to_encoding_does_not_mutate()
{
	MsgStr msg{};
	init_msg(msg, "AB \nC");

	std::string binary = msg.serialize(MsgStr::EncodingType::Binary);
	assertEqual(binary.size(), (size_t)5); // 'A' 'B' ' ' newline-command 'C'
	assertEqual((int)(byte)binary[3], MSGC_NEWLINE + 1);

	assertEqual(msg.s, std::string("AB \nC"));
	assertEqual(msg.encoding_type, MsgStr::EncodingType::Ascii);
	assertEqual(msg.serialize(MsgStr::EncodingType::Ascii), msg.s);

	MsgStr legacy{};
	init_msg(legacy, "AB ", true, MsgStr::EncodingType::Binary);
	// The legacy parser trims trailing spaces...
	assertEqual(legacy.serialize(MsgStr::EncodingType::Ascii), std::string("AB"));
	// ...but the raw data is untouched.
	assertEqual(legacy.s, std::string("AB "));
	assertEqual(legacy.encoding_type, MsgStr::EncodingType::Binary);
}

// A script can rewrite a string while it is being displayed (Game->SetMessage).
// The live iterator's segment indices then refer to a different parse; it must
// end the string safely instead of reading out of bounds.
void test_reparse_mid_iteration_ends_safely()
{
	MsgStr msg{};
	init_msg(msg, R"(x\Speed\1\ \Speed\2\)"); // Literal, Command, Command
	auto it = msg.create_iterator();

	it.next();
	assertEqual(it.character, std::string("x"));
	it.next();
	it.next();
	assertEqual(it.state, MsgStr::iterator::COMMAND); // both commands consumed

	// Re-parse to a string whose next (4th) segment is a command, but which has
	// fewer commands overall than the iterator already consumed.
	init_msg(msg, "ab\ncd\nef"); // Literal, Command, Literal, Command, Literal

	assertTrue(it.next());
	assertTrue(it.done());
}

} // end namespace

TestResults test_msgstr(bool verbose)
{
	TestResults tr{};
	struct { const char* name; void (*fn)(); } tests[] = {
		{ "empty_string_done_on_first_advance", test_empty_string_done_on_first_advance },
		{ "next_after_done_stays_done", test_next_after_done_stays_done },
		{ "spaces_only_string", test_spaces_only_string },
		{ "spaces_only_string_legacy_encoding", test_spaces_only_string_legacy_encoding },
		{ "literal_characters_in_order", test_literal_characters_in_order },
		{ "wrap_word_grouping", test_wrap_word_grouping },
		{ "no_wrap_single_char_words", test_no_wrap_single_char_words },
		{ "command_only_string", test_command_only_string },
		{ "command_as_final_segment", test_command_as_final_segment },
		{ "command_between_literals", test_command_between_literals },
		{ "consecutive_commands", test_consecutive_commands },
		{ "newline_is_a_command", test_newline_is_a_command },
		{ "invalid_command_iterates_as_text", test_invalid_command_iterates_as_text },
		{ "set_buffer_injects_text", test_set_buffer_injects_text },
		{ "set_buffer_empty_steps_past_command", test_set_buffer_empty_steps_past_command },
		{ "post_segment_delay", test_post_segment_delay },
		{ "post_segment_delay_fast", test_post_segment_delay_fast },
		{ "post_segment_delay_forced", test_post_segment_delay_forced },
		{ "peek_within_word", test_peek_within_word },
		{ "peek_at_word_boundary", test_peek_at_word_boundary },
		{ "serialize_to_encoding_does_not_mutate", test_serialize_to_encoding_does_not_mutate },
		{ "reparse_mid_iteration_ends_safely", test_reparse_mid_iteration_ends_safely },
	};

	for (auto& test : tests)
	{
		++tr.total;
		try
		{
			test.fn();
			if (verbose)
				fmt::println("  [PASS] {}", test.name);
		}
		catch (const std::exception& e)
		{
			++tr.failed;
			fmt::println("  [FAIL] {}\n{}", test.name, e.what());
		}
	}

	return tr;
}

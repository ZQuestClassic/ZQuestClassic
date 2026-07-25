// Regression tests for message string handling:
// - blank strings ending immediately (they used to play forever, softlocking
//   the game when shown by a Guy)
// - messagedata->Get() not re-encoding the string in place
// - rewriting a string while it is being displayed
//
// See also the MsgStr::iterator unit tests in src/core/msgstr_test.cpp.

#include "std.zh"
#include "auto/test_runner.zs"

generic script msg_strings
{
	messagedata setMessage(int id, char32[] str)
	{
		messagedata msg = Game->LoadMessageData(id);
		msg->Set(str);
		msg->X = 24;
		msg->Y = 32;
		msg->Sound = 0;
		msg->Next = 0;
		return msg;
	}

	void playMessage(int id)
	{
		Screen->Message(id);
		finishMessage();
	}

	// Holds A until the current message is dismissed.
	void finishMessage()
	{
		int limit = 60 * 10;
		while (Screen->ShowingMessage && limit > 0)
		{
			limit--;
			WaitTo(SCR_TIMING_POST_DRAW);
			Input->Button[CB_A] = true;
			Waitframe();
		}
		Test::Assert(limit > 0, "message never ended");
	}

	void run()
	{
		Test::Init();
		Game->FFRules[qr_ALLOWFASTMSG] = false;
		Game->FFRules[qr_MSGDISAPPEAR] = true;
		// The playground quest defaults this QR to true; the strings below are
		// written in the ascii encoding.
		Game->FFRules[qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING] = false;

		// A blank string ends immediately instead of playing forever, and still
		// follows its Next chain.
		printf("[Test] blank string with a next string\n");
		setMessage(1, "")->Next = 2;
		setMessage(2, "blank ok\\CounterSet\\1\\42\\");
		playMessage(1);
		Test::AssertEqual(Game->Counter[1], 42);

		// Same for a string of only spaces - how 1.90-era quests store blank
		// strings.
		printf("[Test] spaces-only string with a next string\n");
		setMessage(1, "   ")->Next = 2;
		setMessage(2, "spaces ok\\CounterSet\\1\\43\\");
		playMessage(1);
		Test::AssertEqual(Game->Counter[1], 43);

		// A blank string with no next string just ends.
		printf("[Test] blank string with no next string\n");
		setMessage(1, "");
		playMessage(1);

		// Reading a string must not re-encode it in place. With the old-encoding
		// QR enabled, Get() returns the legacy binary encoding, which clamps SCC
		// args to 65023 - but the stored string must keep its full-precision
		// ascii form for the next reader.
		printf("[Test] Get does not re-encode the string\n");
		{
			char32 text[] = "big\\Speed\\100000\\ end";
			messagedata msg = setMessage(1, text);
			char32 buf[100];
			Game->FFRules[qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING] = true;
			msg->Get(buf);
			Game->FFRules[qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING] = false;
			msg->Get(buf);
			Test::Assert(strcmp(buf, text) == 0, "string was downgraded by Get()");
		}

		// Rewriting a string while it is being displayed keeps going: the part
		// already being printed finishes as-is, then the rest of the display
		// comes from the new content. The new string's counter SCC proves the
		// display switched over.
		printf("[Test] rewrite mid-display continues with the new content\n");
		setMessage(1, "\\Speed\\10\\ aaaa bbbb cccc dddd");
		Screen->Message(1);
		Waitframes(30); // Somewhere inside the "aaaa ..." literal.
		setMessage(1, "\\Speed\\10\\ xx\\CounterSet\\1\\44\\ yy");
		finishMessage();
		Test::AssertEqual(Game->Counter[1], 44);

		// If the rewritten string has less content than the display has already
		// gone through, the message must just end (and not read out of bounds).
		printf("[Test] rewrite mid-display to a shorter string\n");
		setMessage(1, "\\Speed\\5\\ aaaa \\Speed\\5\\ bbbb cccc dddd");
		Screen->Message(1);
		Waitframes(60); // Somewhere inside the "bbbb ..." literal.
		setMessage(1, "ab\n\n\ncd");
		finishMessage();

		Test::End();
	}
}

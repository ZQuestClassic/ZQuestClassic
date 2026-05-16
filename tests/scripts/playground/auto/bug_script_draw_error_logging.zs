// https://discord.com/channels/876899628556091432/1500397605929222214

#include "auto/test_runner.zs"

generic script bug_script_draw_error_logging
{
	void run()
	{
		Test::Init();

		bitmap b;
		b->Blit(0, NULL, -1, -1, -1, -1, -1, -1, -1, -1);
		Waitframe();

		Test::End();
	}
}

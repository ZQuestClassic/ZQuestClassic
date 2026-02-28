// https://discord.com/channels/876899628556091432/1477081784976936960

#include "auto/test_runner.zs"

generic script bug_convert_rgb
{
	void run()
	{
		Test::Init();

		Game->FFRules[qr_SCRIPTS_6_BIT_COLOR] = false;

		int buf[4];
		Graphics->ConvertFromRGB(buf, Graphics->CreateRGB(255, 0, 0), CSPACE_CMYK);
		Test::AssertEqual(buf[0], 0);
		Test::AssertEqual(buf[1], 1);
		Test::AssertEqual(buf[2], 1);
		Test::AssertEqual(buf[3], 0);

		Test::End();
	}
}

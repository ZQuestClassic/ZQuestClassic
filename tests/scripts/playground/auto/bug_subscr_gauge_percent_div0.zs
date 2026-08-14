#include "auto/test_runner.zs"
#include "std.zh"

// Repro for SW_GaugePiece dividing by zero while drawing: a gauge using the
// percentage-based animation flags whose counter has a max of 0 crashed the
// game (fixed in 2.55.16 by 74c6ef1a30). The custom counter used here has a
// max of 0, so simply letting the passive subscreen draw for a few frames
// exercises the guarded division.
generic script bug_subscr_gauge_percent_div0
{
	void run()
	{
		Test::Init();

		dmapdata dm = Game->LoadDMapData(Game->GetCurDMap());
		subscreendata sub = Game->LoadPSubData(dm->PassiveSubscreen);
		subscreenpage page = sub->Pages[0];
		subscreenwidget widg = page->CreateWidget(SUBWIDG_MISCGAUGE);
		widg->Counter[0] = CR_CUSTOM1;
		widg->Flags[SUBW_GAUGE_FLAG_ANIMATE_PERCENT] = true;
		widg->Flags[SUBW_GAUGE_FLAG_ANIMATE_UNDER] = true;
		Game->MCounter[CR_CUSTOM1] = 0;

		// Pre-fix this crashed on the first passive subscreen draw.
		Waitframes(10);

		Test::End();
	}
}

#option NO_ERROR_HALT on
#option WARN_DEPRECATED error

global script Active
{
	void run()
	{
		//Read-only vars should warn
		Game->CurScreen = 2;
		Game->CurDMap = 5;
		Hero->IceCombo = 0;
		//Deprecated things error due to #option
		Untype(5);
		TraceNPC(Screen->LoadNPC(0));
		Trace(Screen->ScriptEntry);
	}
}


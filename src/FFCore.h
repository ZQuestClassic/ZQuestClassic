class FFSCriptCore
{
private:
	static bool ffscript_engine;
	static FFSCriptCore *ffengine;
	FFScript()
	{
	}
	
public:
	static FFSCriptCore* getFFEngineRunning();
	void ffscript_function();
	~FFScript()
	{
		ffscript_engine = false;	
	}
};

bool FFSCriptCore::ffscript_engine = false;
FFSCriptCore* FFSCriptCore::ffengine = NULL;
FFSCriptCore* FFSCriptCore::getFFEngineRunning()
{
	if ( !ffscript_engine )
	{
		ffengine = new FFScript();
		ffscript_engine = true;
		return ffengine;
	}
	else
	{
		return ffengine;
	}
}

void FFSCriptCore::ffscript_function()
{
	
}
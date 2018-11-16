struct module
{
	char module_name[2048]; //filepath for current module file
	char quests[10][255]; //first five quests, filenames
	char skipnames[10][255]; //name entry passwords
}; //module

void ZModule::init(bool default)
{
	memset(module, 0, sizeof(module));
	if ( default )
	{
		//module path
		strcpy(module.module_name,get_config_string("ZCMODULE",current_module,"default.zmod"));
		
		//quests
		strcpy(module.quests[0],get_config_string("ZCMODULE",first_qst,"1st.qst"));
		strcpy(module.quests[1],get_config_string("ZCMODULE",second_qst,"2nd.qst"));
		strcpy(module.quests[2],get_config_string("ZCMODULE",third_qst,"3rd.qst"));
		strcpy(module.quests[3],get_config_string("ZCMODULE",fourth_qst,"4th.qst"));
		strcpy(module.quests[4],get_config_string("ZCMODULE",fifth_qst,"5th.qst"));
		
		//quest skip names
		strcpy(module.skipnames[0],get_config_string("ZCMODULE",first_qst_skip," "));
		strcpy(module.skipnames[1],get_config_string("ZCMODULE",second_qst_skip,"ZELDA"));
		strcpy(module.skipnames[2],get_config_string("ZCMODULE",third_qst_skip,"ALPHA"));
		strcpy(module.skipnames[3],get_config_string("ZCMODULE",fourth_qst_skip,"GANON"));
		strcpy(module.skipnames[4],get_config_string("ZCMODULE",fifth_qst_skip,"JEAN"));
		
		
	}
}

void ZModule::load(bool zquest)
{
	set_config_file(module.module_name);
	//load config settings
	if ( zquest )
	{
		//load ZQuest section data
		set_config_file("zquest.cfg"); //shift back when done
	}
	else
	{
		//load ZC section data
		set_config_file("zc.cfg"); //shift back when done
	}
	
}

class ZModule
{
	public:
		void init(bool default);
		void load(bool zquest);
		
	private:
};


struct zcmodule
{
	char module_name[2048]; //filepath for current zcmodule file
	char quests[10][255]; //first five quests, filenames
	char skipnames[10][255]; //name entry passwords
}; //zcmodule

zcmodule zcm;

void ZModule::init(bool default)
{
	memset(zcm, 0, sizeof(zcm));
	if ( default )
	{
		//zcm path
		strcpy(zcm->module_name,get_config_string("ZCMODULE",current_module,"default.zmod"));
		
		//quests
		strcpy(zcm->quests[0],get_config_string("ZCMODULE",first_qst,"1st.qst"));
		strcpy(zcm->quests[1],get_config_string("ZCMODULE",second_qst,"2nd.qst"));
		strcpy(zcm->quests[2],get_config_string("ZCMODULE",third_qst,"3rd.qst"));
		strcpy(zcm->quests[3],get_config_string("ZCMODULE",fourth_qst,"4th.qst"));
		strcpy(zcm->quests[4],get_config_string("ZCMODULE",fifth_qst,"5th.qst"));
		
		//quest skip names
		strcpy(zcm->skipnames[0],get_config_string("ZCMODULE",first_qst_skip," "));
		strcpy(zcm->skipnames[1],get_config_string("ZCMODULE",second_qst_skip,"ZELDA"));
		strcpy(zcm->skipnames[2],get_config_string("ZCMODULE",third_qst_skip,"ALPHA"));
		strcpy(zcm->skipnames[3],get_config_string("ZCMODULE",fourth_qst_skip,"GANON"));
		strcpy(zcm->skipnames[4],get_config_string("ZCMODULE",fifth_qst_skip,"JEAN"));
		
		
	}
}

void ZModule::load(bool zquest)
{
	set_config_file(zcm->module_name);
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


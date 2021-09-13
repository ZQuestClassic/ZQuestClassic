#include "parser/ZScript.h"
#include "ffscript.h"
#include <string>

FFScript FFCore;
std::vector<std::string> ZQincludePaths;
byte quest_rules[QUESTRULES_NEW_SIZE];

int get_bit(byte *bitstr,int bit)
{
    bitstr += bit>>3;
    return ((*bitstr) >> (bit&7))&1;
}
void box_out(const char *msg) {}
void box_out_nl(const char *msg) {}
void box_eol() {}

int used_switch(int argc,char *argv[],const char *s)
{
    // assumes a switch won't be in argv[0]
    for(int i=1; i<argc; i++)
        if(stricmp(argv[i],s)==0)
            return i;

    return 0;
}

int compile(std::string script_path)
{
    printf("compiling %s\n", script_path.c_str());

    // copy to tmp file
    std::string zScript;
    FILE *zscript = fopen(script_path.c_str(),"r");
    if(zscript == NULL)
    {
        printf("Error: Cannot open specified file!\n");
        return 1;
    }

    char c = fgetc(zscript);
    while(!feof(zscript))
    {
        zScript += c;
        c = fgetc(zscript);
    }
    fclose(zscript);

    FILE *tempfile = fopen("tmp","w");
    if(!tempfile)
    {
        printf("Error: Unable to create a temporary file in current directory!\n");
        return 1;
    }
    fwrite(zScript.c_str(), sizeof(char), zScript.size(), tempfile);
    fclose(tempfile);

    boost::movelib::unique_ptr<ZScript::ScriptsData> result(ZScript::compile("tmp"));
    unlink("tmp");

    return 0;
}

void updateIncludePaths()
{
	FILE* f = fopen("includepaths.txt", "r");
	char includePathString[MAX_INCLUDE_PATH_CHARS] = {0};
	if(f)
	{
		int pos = 0;
		int c;
		do
		{
			c = fgetc(f);
			if(c!=EOF) 
				includePathString[pos++] = c;
		}
		while(c!=EOF && pos<MAX_INCLUDE_PATH_CHARS);
		if(pos<MAX_INCLUDE_PATH_CHARS)
			includePathString[pos] = '\0';
		includePathString[MAX_INCLUDE_PATH_CHARS-1] = '\0';
		fclose(f);
	}
	else strcpy(includePathString, "include/;headers/;scripts/;");
	ZQincludePaths.clear();
	int pos = 0; int pathnumber = 0;
	for ( int q = 0; includePathString[pos]; ++q )
	{
		int dest = 0;
		char buf[2048] = {0};
		while(includePathString[pos] != ';' && includePathString[pos])
		{
			buf[dest] = includePathString[pos];
			++pos;
			++dest;
		}
		++pos;
		std::string str(buf);
		ZQincludePaths.push_back(str);
	}
}

int main(int argc, char **argv)
{
    int script_path_index = used_switch(argc, argv, "-input");
    if (!script_path_index) {
        printf("Error: missing required flag: -input\n");
        return 1;
    }
    std::string script_path = argv[script_path_index + 1];

    allegro_init();
    set_config_file("zscript.cfg");
    memset(FFCore.scriptRunString,0,sizeof(FFCore.scriptRunString));
	strcpy(FFCore.scriptRunString, get_config_string("Compiler","run_string","run"));
	updateIncludePaths();
    // Any errors will be printed to stdout.
    int res = compile(script_path);
    allegro_exit();
    return res;
}
END_OF_MAIN()

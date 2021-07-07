#include "parser/zscript.h"
#include "ffscript.h"
#include <string>

FFScript FFCore;
char ZQincludePaths[MAX_INCLUDE_PATHS][512];
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

int main(int argc, char **argv)
{
    int script_path_index = used_switch(argc, argv, "-input");
    if (!script_path_index) {
        printf("Error: missing required flag: -input\n");
        return 1;
    }
    std::string script_path = argv[script_path_index + 1];

    allegro_init();
    memset(FFCore.scriptRunString,0,sizeof(FFCore.scriptRunString));
	strcpy(FFCore.scriptRunString,"run");

    // Any errors will be printed to stdout.
    int res = compile(script_path);
    allegro_exit();
    return res;
}
END_OF_MAIN()

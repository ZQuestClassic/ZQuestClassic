#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sstream>

#include "zc_malloc.h"
#include "ffasm.h"
#include "zquest.h"
#include "zsys.h"
#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_UNIX
#define strnicmp strncasecmp
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif

using namespace std;
using namespace ZAsm;

extern char *datapath, *temppath;


long ffparse(char *string)
{
    //return int(atof(string)*10000);
    
    //this function below isn't working too well yet
    //clean_numeric_string(string);
    double negcheck = atof(string);
    
    //if no decimal point, ascii to int conversion
    char *ptr=strchr(string, '.');
    
    if(!ptr)
    {
        return atoi(string)*10000;
    }
    
    long ret=0;
    char *tempstring1;
    tempstring1=(char *)zc_malloc(strlen(string)+5);
    sprintf(tempstring1, string);
    
    for(int i=0; i<4; ++i)
    {
        tempstring1[strlen(string)+i]='0';
    }
    
    ptr=strchr(tempstring1, '.');
    *ptr=0;
    ret=atoi(tempstring1)*10000;
    
    ++ptr;
    char *ptr2=ptr;
    ptr2+=4;
    *ptr2=0;
    
    if(negcheck<0)
        ret-=atoi(ptr);
    else ret+=atoi(ptr);
    
    zc_free(tempstring1);
    return ret;
}

bool ffcheck(char *arg)
{

    for(int i=0; i<0x100; i++)
    {
        if(arg[i]!='\0')
        {
            if(i==0)
            {
                if(arg[i]!='-' && arg[i]!='.' && !(arg[i] >= '0' && arg[i] <='9'))
                    return false;
            }
            else
            {
                if(arg[i]!='.' && !(arg[i] >= '0' && arg[i] <='9'))
                    return false;
            }
        }
        else
        {
            i=0x100;
        }
    }
    
    return true;
}

char labels[65536][80];
int lines[65536];
int numlines;

int parse_script(ZAsmScript &script, int type)
{
    if(!getname("Import Script (.txt)","txt",NULL,datapath,false))
        return D_CLOSE;

	script.version = ZASM_VERSION;
	script.type = type;
	delete[] script.name;
	script.name_len = 14;
	script.name = new char[14];
	strcpy(script.name, "Imported ZASM");
        
    int ret = parse_script_file(&script.commands,temppath, true);
	int len = 0;
	while (script.commands[len].command != 0xFFFF)
		len++;
	len++;
	script.commands_len = len;
	return ret;
}

int parse_script_file(zasm **script, const char *path, bool report_success)
{
    saved=false;
    FILE *fscript = fopen(path,"rb");
    char *buffer = new char[0x400];
    char *combuf = new char[0x100];
    char *arg1buf = new char[0x100];
    char *arg2buf = new char[0x100];
    bool stop=false;
    bool success=true;
    numlines = 0;
    int num_commands;
    
    for(int i=0;; i++)
    {
        buffer[0]=0;
        
        if(stop)
        {
            num_commands=i+1;
            break;
        }
        
        for(int j=0; j<0x400; j++)
        {
            char temp;
            temp = getc(fscript);
            
            if(feof(fscript))
            {
                stop=true;
                buffer[j]='\0';
                j=0x400;
                ungetc(temp,fscript);
            }
            else
            {
                ungetc(temp,fscript);
                buffer[j] = getc(fscript);
                
                if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
                {
                    if(buffer[j] == '\n')
                    {
                        buffer[j] = '\0';
                        j=0x400;
                    }
                    else
                    {
                        while(getc(fscript)!='\n')
                        {
                            if(feof(fscript))
                            {
                                stop=true;
                                break;
                            }
                        }
                        
                        buffer[j] = '\0';
                        j=0x400;
                    }
                }
            }
        }
        
        int k=0;
        
        while(buffer[k] == ' ' || buffer[k] == '\t') k++;
        
        if(buffer[k] == '\0')
        {
            i--;
            continue;
        }
        
        k=0;
        
        if(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0')
        {
            while(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0')
            {
                labels[numlines][k] = buffer[k];
                k++;
            }
            
            labels[numlines][k] = '\0';
            lines[numlines] = i;
            numlines++;
        }
    }
    
    fseek(fscript, 0, SEEK_SET);
    stop = false;
    
    if((*script)!=NULL) delete [](*script);
    
    (*script) = new zasm[num_commands];
    
    for(int i=0; i<num_commands; i++)
    {
        if(stop)
        {
            (*script)[i].command = 0xFFFF;
            break;
        }
        else
        {
            /*
                  sprintf(buffer, "");
                  sprintf(combuf, "");
                  sprintf(arg1buf, "");
                  sprintf(arg2buf, "");
            */
            buffer[0]=0;
            combuf[0]=0;
            arg1buf[0]=0;
            arg2buf[0]=0;
            
            for(int j=0; j<0x400; j++)
            {
                char temp;
                temp = getc(fscript);
                
                if(feof(fscript))
                {
                    stop=true;
                    buffer[j]='\0';
                    j=0x400;
                    ungetc(temp,fscript);
                }
                else
                {
                    ungetc(temp,fscript);
                    buffer[j] = getc(fscript);
                    
                    if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
                    {
                        if(buffer[j] == '\n')
                        {
                            buffer[j] = '\0';
                            j=0x400;
                        }
                        else
                        {
                            while(getc(fscript)!='\n')
                            {
                                if(feof(fscript))
                                {
                                    stop=true;
                                    break;
                                }
                            }
                            
                            buffer[j] = '\0';
                            j=0x400;
                        }
                    }
                }
            }
            
            int k=0, l=0;
            
            while(buffer[k] == ' ' || buffer[k] == '\t') k++;
            
            if(buffer[k] == '\0')
            {
                i--;
                continue;
            }
            
            k=0;
            
            while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0') k++;
            
            while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0')  k++;
            
            while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
            {
                combuf[l] = buffer[k];
                k++;
                l++;
            }
            
            combuf[l] = '\0';
            l=0;
            
            while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0') k++;
            
            while(buffer[k] != ',' && buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
            {
                arg1buf[l] = buffer[k];
                k++;
                l++;
            }
            
            arg1buf[l] = '\0';
            l=0;
            
            while((buffer[k] == ' ' || buffer[k] == '\t' || buffer[k] == ',') && buffer[k] != '\0') k++;
            
            while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
            {
                arg2buf[l] = buffer[k];
                k++;
                l++;
            }
            
            arg2buf[l] = '\0';
            int parse_err;
            
            if(!(parse_script_section(combuf, arg1buf, arg2buf, script, i, parse_err)))
            {
                char buf[80],buf2[80],buf3[80],name[13];
                const char* errstrbuf[] =
                {
                    "invalid instruction!",
                    "parameter 1 invalid!",
                    "parameter 2 invalid!"
                };
                extract_name(temppath,name,FILENAME8_3);
                sprintf(buf,"Unable to parse instruction %d from script %s",i+1,name);
                sprintf(buf2,"The error was: %s",errstrbuf[parse_err]);
                sprintf(buf3,"The command was (%s) (%s,%s)",combuf,arg1buf,arg2buf);
                jwin_alert("Error",buf,buf2,buf3,"O&K",NULL,'k',0,lfont);
                stop=true;
                success=false;
                (*script)[0].command = 0xFFFF;
            }
        }
    }
    
    if(report_success && success) //(!stop) // stop is never true here
    {
        char buf[80],name[13];
        extract_name(temppath,name,FILENAME8_3);
        sprintf(buf,"Script %s has been parsed",name);
        jwin_alert("Success",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    delete [] buffer;
    delete [] combuf;
    delete [] arg1buf;
    delete [] arg2buf;
    fclose(fscript);
    return success?D_O_K:D_CLOSE;
}

int set_argument(char* argName, zasm** script, int scriptIndex, int argIndex)
{
	zasm& command = (*script)[scriptIndex];
	int32_t& argument = argIndex ? command.arg2 : command.arg1;

	Variable variable = getVariable(argName);
	if (variable.isNull()) return 0;

	argument = variable.getId();
	return 1;
}

#define ERR_INSTRUCTION 0
#define ERR_PARAM1 1
#define ERR_PARAM2 2

int parse_script_section(
		char* commandName, char* firstArg, char* secondArg,
		zasm **script, int scriptIndex, int& retcode)
{
	zasm& command = (*script)[scriptIndex];
    command.arg1 = 0;
    command.arg2 = 0;

    CommandDef* commandDef = getCommandDef(commandName);
    if (!commandDef)
    {
	    retcode = ERR_INSTRUCTION;
	    return 0;
    }
    command.command = commandDef->getId();

    // Jump Command
    if (commandDef->getFirstArg() == CommandDef::ArgLabel)
    {
	    bool nomatch = true;
	    for (int j = 0; j < numlines; ++j)
	    {
		    if (stricmp(firstArg, labels[j]) == 0)
		    {
			    command.arg1 = lines[j];
			    nomatch = false;
			    j = numlines;
		    }
	    }
                
	    if (nomatch)
	    {
		    command.arg1 = atoi(firstArg) - 1;
	    }
                
	    if (strnicmp(commandName, "LOOP", 4) == 0)
	    {
		    // This should NEVER happen with a loop, as arg2 needs to be
		    // a variable.
		    if (commandDef->getSecondArg() != CommandDef::ArgRegister)
		    {
			    if (!ffcheck(secondArg))
			    {
				    retcode = ERR_PARAM2;
				    return 0;
			    }
			    command.arg2 = ffparse(secondArg);
		    }
		    else
		    {
			    if (!set_argument(secondArg, script, scriptIndex, 1))
			    {
				    retcode = ERR_PARAM2;
				    return 0;
			    }
		    }
	    }
    }
    // Normal Command
    else
    {
	    if (commandDef->hasFirstArg())
	    {
		    if (commandDef->getFirstArg() == CommandDef::ArgValue)
		    {
			    if (!ffcheck(firstArg))
			    {
				    retcode = ERR_PARAM1;
				    return 0;
			    }
			    command.arg1 = ffparse(firstArg);
		    }
		    else
		    {
			    if (!set_argument(firstArg, script, scriptIndex, 0))
			    {
				    retcode = ERR_PARAM1;
				    return 0;
			    }
		    }
                    
		    if (commandDef->hasSecondArg())
		    {
			    if (commandDef->getSecondArg() == CommandDef::ArgValue)
			    {
				    if (!ffcheck(secondArg))
				    {
					    retcode = ERR_PARAM2;
					    return 0;
				    }
				    command.arg2 = ffparse(secondArg);
			    }
			    else
			    {
				    if (!set_argument(secondArg, script, scriptIndex, 1))
				    {
					    retcode = ERR_PARAM2;
					    return 0;
				    }
			    }
		    }
	    }
    }

    return 1;
}

std::string to_string(zasm const& instruction)
{
	ostringstream out;
	char buf[32];
	CommandDef* commandDef =
		getCommandDef(CommandId(instruction.command));

	// Command Name.
	out << commandDef->getName();
	if (!commandDef->hasFirstArg()) return out.str();
	
	// First Argument.
	out << " ";
	// If it's a variable just print it out.
	if (commandDef->getFirstArg() == CommandDef::ArgRegister)
		out << getVariable(VariableId(instruction.arg1)).toString();
	// If it's a label, don't move decimal point.
	else if (commandDef->getFirstArg() == CommandDef::ArgLabel)
		out << instruction.arg1;
	// Otherwise, do decimal adjustment.
	else if (instruction.arg1 % 10000 == 0)
		out << instruction.arg1 / 10000;
	else
		out << instruction.arg1 * 0.0001;
	if (!commandDef->hasSecondArg()) return out.str();
	
	// Second Argument.
	out << ", ";
	// If it's a variable just print it out.
	if (commandDef->getSecondArg() == CommandDef::ArgRegister)
		out << getVariable(VariableId(instruction.arg2)).toString();
	// Otherwise, do decimal adjustment.
	else if (instruction.arg2 % 10000 == 0)
		out << instruction.arg2 / 10000;
	else
		out << instruction.arg2 * 0.0001;
	return out.str();
}

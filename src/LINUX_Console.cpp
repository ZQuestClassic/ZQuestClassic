#ifdef _WIN32
	#include "ConsoleLogger.h"
#else //Unix
	#include <fcntl.h>
	#include <unistd.h>
	#include <iostream>
	#include <sstream>
	int32_t pt = 0;
	char* ptname = NULL;
	std::ostringstream lxconsole_oss;
#endif

//Unix
#ifndef _WIN32

    { // Let's try making a console for Linux -Z
	pt = posix_openpt(O_RDWR);
	if (pt == -1)
	{
		Z_error_fatal("Could not open pseudo terminal; error number: %d.\n", errno);
		goto no_lx_console;
	}
	ptname = ptsname(pt);
	if (!ptname)
	{
		Z_error_fatal("Could not get pseudo terminal device name.\n");
		close(pt);
		goto no_lx_console;
	}

	if (unlockpt(pt) == -1)
	{
		Z_error_fatal("Could not get pseudo terminal device name.\n");
		close(pt);
		goto no_lx_console;
	}

	lxconsole_oss << "xterm -S" << (strrchr(ptname, '/')+1) << "/" << pt << " &";
	system(lxconsole_oss.str().c_str());

	int32_t xterm_fd = open(ptname,O_RDWR);
	{
		char c = 0; int32_t tries = 10000; 
		do 
		{
			read(xterm_fd, &c, 1); 
			--tries;
		} while (c!='\n' && tries > 0);
	}

	if (dup2(pt, 1) <0)
	{
		Z_error_fatal("Could not redirect standard output.\n");
		close(pt);
		goto no_lx_console;
	}
	if (dup2(pt, 2) <0)
	{
		Z_error_fatal("Could not redirect standard error output.\n");
		close(pt);
		goto no_lx_console;
	}
    } //this is in a block because I want it in a block. -Z
    
    no_lx_console:
    {
	    //Z_error_fatal("Could not open Linux console.\n");
    }
    
    
	std::cout << "\n       _____   ____                  __ \n";
	std::cout << "      /__  /  / __ \\__  _____  _____/ /_\n";
	std::cout << "        / /  / / / / / / / _ \\/ ___/ __/\n";
	std::cout << "       / /__/ /_/ / /_/ /  __(__  ) /_ \n";
	std::cout << "      /____/\\___\\_\\__,_/\\___/____/\\__/\n\n";
	
	std::cout << "Quest Data Logging & ZScript Debug Console\n";
	std::cout << "ZConsole for Linux\n\n";
    
	if ( FFCore.getQuestHeaderInfo(vZelda) > 0 )
	{
		printf("Quest Made in ZC Version %x, Build %d\n", FFCore.getQuestHeaderInfo(vZelda), FFCore.getQuestHeaderInfo(vBuild));
	}
	else
	{
		printf("%s, Version %s\n", ZC_PLAYER_NAME, ZC_PLAYER_V);
	}
	//std::cerr << "Test cerr\n\n";
	std::cin.ignore(1);
#endif
    
// ConsoleLogger_Demo.cpp : Defines the entry point for the console application.
//

#include "ConsoleLogger.h"

int main(int argc, char* argv[])
{
	printf ("Hi , i'm the main-process , and  i'm going to show you 2 more consoles...");
	CConsoleLoggerEx coloured_console;
	coloured_console.Create("This is the coloured console");
	
	coloured_console.cls(CConsoleLoggerEx::COLOR_BACKGROUND_RED);
	coloured_console.gotoxy(10,10);
	coloured_console.cprintf( CConsoleLoggerEx::COLOR_WHITE| CConsoleLoggerEx::COLOR_BACKGROUND_BLUE,"White on Blue");


	CConsoleLoggerEx coloured_console1;
	coloured_console1.Create("This is the another console");
	
	coloured_console1.cprintf( CConsoleLoggerEx::COLOR_RED | CConsoleLoggerEx::COLOR_BACKGROUND_BLACK,"Another console");


	getchar();
	
	
	return 0;
}


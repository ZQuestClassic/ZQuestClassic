@echo off
if '%1' == 'zquest-w' goto zquest
if '%1' == 'zelda-w' goto zelda
echo Usage:
echo test (program) [options]
goto end
:zquest
if exist zquest-w.exe del zquest-w.exe
make
zquest-w %2 %3 %4 %5 %6 %7 %8 %9
goto end
:zelda
if exist zelda-w.exe del zelda-w.exe
make
zelda-w %2 %3 %4 %5 %6 %7 %8 %9
:end
echo on

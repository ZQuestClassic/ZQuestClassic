@echo off
md %1
copy *.c %1
copy *.h %1
copy *.cpp %1
copy *.def %1
copy makefile* %1
copy z*.g* %1
copy backup.bat %1


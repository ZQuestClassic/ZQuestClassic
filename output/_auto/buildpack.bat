@REM TODO: delete this file and output/ folder.

@echo off
cd ..\..
SETLOCAL
set src=.

if "%~2"=="" (
	set rel=%src%\Release
)else (
	set rel=%~2
)

python scripts\package.py --build_folder "%rel%"

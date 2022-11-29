@echo off
cd ..\..
SETLOCAL
set src=.
set out=%src%\output
set log=%out%\_auto\extrapack_log.txt
set extrapack=%out%\_auto\extrapack
set util=%out%\utilities
set addon=%out%\addons

if exist "%extrapack%" (
	echo Clearing old extrapack folder...
	echo Clearing old extrapack folder...>%log%
	rmdir /S /Q "%extrapack%"
)
echo Copying '\output\utilities'...
echo Copying '\output\utilities'...>>%log%
xcopy /e /y /i "%util%" "%extrapack%\utilities" >>%log%

echo Copying '\output\addons'...
echo Copying '\output\addons'...>>%log%
xcopy /e /y /i "%addon%" "%extrapack%\addons" >>%log%

set "ch=%~1"
if "%ch%"=="Y" goto :zip
if "%ch%"=="N" goto :close

choice /c YN /N /T 20 /D Y /M "Create .zip? (Y/N)"

if %ERRORLEVEL% NEQ 1 goto :close

:zip
echo Zipping...
echo Zipping...>>%log%
if exist "C:\Program Files\7-Zip\7z.exe" (

	if exist "%extrapack%.zip" (
		del "%extrapack%.zip"
	)

	"C:\Program Files\7-Zip\7z.exe" a -tzip "%extrapack%.zip" -r "%extrapack%\*" -mx9

	goto end
)

echo No ZIP Program Found!
echo No ZIP Program Found! >>%log%
:end
rem Only pause if called without params
if "%ch%"=="" pause
:close

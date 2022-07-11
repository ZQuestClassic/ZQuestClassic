@echo off
cd ..\..
SETLOCAL
set src=.
set out=%src%\output
set log=%out%\_auto\buildpack_log.txt
set nb=%out%\_auto\buildpack
set rel=%src%\Release
set incl=%out%\include
set docs=%out%\docs
set theme=%out%\themes
set tsets=%out%\tilesets
set conf=%out%\config
set mod=%out%\modules
set pack=%out%\package

if exist "%nb%" (
	echo Clearing old buildpack folder...
	echo Clearing old buildpack folder...>%log%
	rmdir /S /Q "%nb%"
)

echo Copying '\output\package' files...
echo Copying '\output\package' files...>%log%
xcopy /e /y /i "%pack%" "%nb%" >>%log%

echo Copying '\Release' - zelda.exe, zquest.exe, zscript.exe, zlauncher.exe, *.dll...
echo Copying '\Release' - zelda.exe, zquest.exe, zscript.exe, zlauncher.exe, zconsole.exe, *.dll...>>%log%
copy /y "%rel%\zelda.exe" "%nb%\zelda.exe" >>%log%
copy /y "%rel%\zquest.exe" "%nb%\zquest.exe" >>%log%
copy /y "%rel%\zscript.exe" "%nb%\zscript.exe" >>%log%
copy /y "%rel%\zlauncher.exe" "%nb%\zlauncher.exe" >>%log%
copy /y "%rel%\zconsole.exe" "%nb%\zconsole.exe" >>%log%
copy /y "%rel%\crashpad_handler.exe" "%nb%\crashpad_handler.exe" >>%log%
copy /y "%rel%\*.dll" "%nb%" >>%log%

echo Copying 'changelog.txt'...
echo Copying 'changelog.txt'...>>%log%
copy /y "%src%\changelog.txt" "%nb%\changelog.txt" >>%log%

echo Copying '\output\include'...
echo Copying '\output\include'...>>%log%
xcopy /e /y /i "%incl%" "%nb%\include" >>%log%

echo Copying '\output\docs'...
echo Copying '\output\docs'...>>%log%
xcopy /e /y /i "%docs%" "%nb%\docs" >>%log%

echo Copying '\output\themes'...
echo Copying '\output\themes'...>>%log%
xcopy /e /y /i "%theme%" "%nb%\themes" >>%log%

echo Copying '\output\tilesets'...
echo Copying '\output\tilesets'...>>%log%
xcopy /e /y /i "%tsets%" "%nb%\tilesets" >>%log%

echo Copying '\output\configs'...
echo Copying '\output\configs'...>>%log%
xcopy /e /y /i "%conf%" "%nb%" >>%log%

echo Copying '\output\modules'...
echo Copying '\output\modules'...>>%log%
xcopy /e /y /i "%mod%" "%nb%\modules" >>%log%

set "ch=%~1"
if "%ch%"=="Y" goto :zip
if "%ch%"=="N" goto :close

choice /c YN /N /T 20 /D Y /M "Create .zip? (Y/N)"

if %ERRORLEVEL% NEQ 1 goto :close

:zip
echo Zipping...
echo Zipping...>>%log%
if exist "C:\Program Files\7-Zip\7z.exe" (
	
	if exist "%nb%.zip" (
		del "%nb%.zip"
	)
	
	"C:\Program Files\7-Zip\7z.exe" a -tzip "%nb%.zip" -r "%nb%\*" -mx9
	
	goto end
)

echo No ZIP Program Found!
echo No ZIP Program Found! >>%log%
:end
rem Only pause if called without params
if "%ch%"=="" pause
:close

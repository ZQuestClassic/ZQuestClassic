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
set util=%out%\utilities
set conf=%out%\config
set mod=%out%\modules
set pack=%out%\package

echo Copying '\output\package' files...
echo Copying '\output\package' files...>%log%
xcopy /e /y /i "%pack%" "%nb%" >>%log%

echo Copying '\Release' - zelda.exe, zquest.exe, zcsound.dll...
echo Copying '\Release' - zelda.exe, zquest.exe, zcsound.dll...>>%log%
copy /y "%rel%\zelda.exe" "%nb%\zelda.exe" >>%log%
copy /y "%rel%\zquest.exe" "%nb%\zquest.exe" >>%log%
copy /y "%rel%\zcsound.dll" "%nb%\zcsound.dll" >>%log%

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

echo Copying '\output\utilities'...
echo Copying '\output\utilities'...>>%log%
xcopy /e /y /i "%util%" "%nb%\utilities" >>%log%

echo Copying '\output\configs'...
echo Copying '\output\configs'...>>%log%
xcopy /e /y /i "%conf%" "%nb%" >>%log%

echo Copying '\output\modules'...
echo Copying '\output\modules'...>>%log%
xcopy /e /y /i "%mod%" "%nb%\modules" >>%log%

choice /c YN /N /T 20 /D Y /M "Create .zip? (Y/N)"

if %ERRORLEVEL% NEQ 1 goto close

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
pause
:close

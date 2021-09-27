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

echo Copying '\output\include'...
echo Copying '\output\include'...>>%log%
xcopy /e /y /i "%incl%" "%nb%\include" >>%log%

echo Copying '\output\docs'...
echo Copying '\output\docs'...>>%log%
xcopy /e /y /i "%docs%" "%nb%\docs" >>%log%

echo Copying '\output\themes'...
echo Copying '\output\themes'...>>%log%
xcopy /e /y /i "%theme%" "%nb%\themes" >>%log%

echo Copying '\output\utilities'...
echo Copying '\output\utilities'...>>%log%
xcopy /e /y /i "%util%" "%nb%\utilities" >>%log%

echo Copying '\output\configs'...
echo Copying '\output\configs'...>>%log%
xcopy /e /y /i "%conf%" "%nb%" >>%log%

echo Copying '\output\modules'...
echo Copying '\output\modules'...>>%log%
xcopy /e /y /i "%mod%" "%nb%\modules" >>%log%

pause


@echo off
c:\lazarus\fpc\2.0.4\bin\i386-win32\windres.exe --include c:\lazarus\fpc\2.0.4\bin\i386-win32\ -O coff -o zcl.or zcl.res
if errorlevel 1 goto linkend
c:\lazarus\fpc\2.0.4\bin\i386-win32\windres.exe --include c:\lazarus\fpc\2.0.4\bin\i386-win32\ -O coff -o zcl.or zcl.res
if errorlevel 1 goto linkend
SET THEFILE=ZCL.exe
echo Linking %THEFILE%
c:\lazarus\fpc\2.0.4\bin\i386-win32\ld.exe -b pe-i386 -m i386pe  -s --subsystem windows   -o ZCL.exe link.res
if errorlevel 1 goto linkend
c:\lazarus\fpc\2.0.4\bin\i386-win32\postw32.exe --subsystem gui --input ZCL.exe --stack 262144
if errorlevel 1 goto linkend
goto end
:asmend
echo An error occured while assembling %THEFILE%
goto end
:linkend
echo An error occured while linking %THEFILE%
:end

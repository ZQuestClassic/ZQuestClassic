echo off
cls
echo Optimize Script
strip --strip-all ZCL.exe
upx ZCL.exe
rename ZCL.exe zlaunch-w.exe
Echo Done
pause

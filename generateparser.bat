@echo off
cd parser\
echo Running ffscript.ypp through Bison
bison -b y -v -d ffscript.ypp
gramdiag y.output > ffscript.report
Pause
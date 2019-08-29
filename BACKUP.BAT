@echo off
md %1
md %1\parser
copy *.c %1
copy *.h %1
copy *.cpp %1
copy Makefile %1
copy makefile.inc %1
copy parser\*.* %1\parser
del parser\*.o


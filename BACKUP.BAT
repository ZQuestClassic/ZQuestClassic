@echo off
md %1
md %1\parser
copy *.h %1
copy *.cpp %1
copy Makefile %1
copy parser\*.* %1\parser
del %1\parser\*.o


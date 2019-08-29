@echo off
md %1
copy *.c %1
copy *.h %1
copy *.cpp %1
copy Makefile %1
copy makefile.inc %1


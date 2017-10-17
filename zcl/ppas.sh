#!/bin/sh
DoExitAsm ()
{ echo "An error occurred while assembling $1"; exit 1; }
DoExitLink ()
{ echo "An error occurred while linking $1"; exit 1; }
/usr/bin/fpcres -o zcl.or -i zcl.res
if [ $? != 0 ]; then DoExitLink ; fi
/usr/bin/fpcres -o zcl.or -i zcl.res
if [ $? != 0 ]; then DoExitLink ; fi
echo Linking ZCL
/usr/bin/ld -b elf32-i386 -m elf_i386  -dynamic-linker=/lib/ld-linux.so.2   -s -L. -o ZCL link.res
if [ $? != 0 ]; then DoExitLink ZCL; fi
/usr/bin/fpcres  -f -i ZCL
if [ $? != 0 ]; then DoExitLink ; fi


                           AllegroMP3 v2.0.5
                          ===================

               AllegroMP3 (c) 2001 - 2006 Javier Gonzalez

MP3 Decoder part of mpglib that is part of mpg123 available at www.mpg123.com
              see even more credits at decoder/AUTHORS file


AllegroMP3 is an Allegro wrapper for the mpglib MP3 decoder part of mpg123
AllegroMP3 is distributed under the GNU Lesser General Public License
Version 2.1 (see file COPYING.txt).

The full source code (very slightly modified to make it compatible
with Allegro and DOS) of this one is available in the decoder directory.


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



AllegroMP3 uses code from:
  - mpglib par of mpg123 (www.mpg123.com)


IMPORTANT NOTE ABOUT LGPL: Due to the use of the LGPLed mpglib code part of
  mpg123, I cannot change the AllegroMP3 license. Basically to agree to the
  LGPL conditions, if you *STATIC LINK* the library to your code you will
  have to include either the object files or the sources of your program.
  This restriction does NOT apply to the dynamic link library version (DLL
  on Windows, .so on Linux (by the way, there is no option in the makefile
  to make a dynamic link library in Linux, contributions welcomed)).
  For any more info on why all this happens, check the section 6 in the
  COPYING.txt file, but briefly it is that you must always let your users
  the possibility to use a newer version of the library with your program.
  And if you ask me, it is stupid to me.


IMPORTANT NOTE FOR ALL VERSIONS: If you want to use the DLL please link
  to the import library (almp3dll.lib in MSVC or libalmp3dll.a for the
  other systems (except DJGPP of course)) *and* define ALMP3_DLL
  *before* including almp3.h, this way for example:
    #define ALMP3_DLL
    #include <almp3.h>
  If you wish to use the STATIC library just don't define ALMP3_DLL

  Also please note the examples use the static link library, so to build
  them you will need to build the static link library first.


How to compile the library and example code:

  If you are running under a un*x system first run fixunix.sh
  If you want to use the DOS/Windows the pack is ready to be used
  as is right now, no conversion needed. fixdos.bat is only provided
  to be used if you used fixunix.sh to change file formats to unix one
  and you want them to be DOS format back again. Note that to use fixdos.bat
  you will need utod.exe.


  In case you are using GCC (DJGPP, MINGW32):

  Then edit the makefile and uncomment the target you want to compile to then run in the command
  line "make" and that's all.

  It will generate a static link library (libalmp3.a) ready to use in your
  programs, or a dynamic link library (almp3.dll and libalmp3dll.a import
  lib in the Mingw32 case).

  After that, if you want to compile the examples, get inside the directory
  examples, edit the makefile uncommenting the target you want to compile to
  and type "make".


  In case you are using Microsoft Visual C++:

  Open the project file and build whatever you choose. There are two options:
    1) To build the library as a static link library (almp3_static), that will
       generate the almp3.lib file.
    2) To build the library as a dynamic link libray (almp3_dll) that will
       generate the almp3dll.lib and almp3.dll files.


How to contact me:

  xaviergonz@hotmail.com



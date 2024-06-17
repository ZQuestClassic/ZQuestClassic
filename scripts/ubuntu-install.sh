#!/bin/bash

cd "$(dirname "$0")" || exit 1

if [[ ! "$UID" == "0" ]];
then
	echo "$0 must be run as root!"
	exit 2;
fi

APP="${1-apt}"
AINST="${APP} install -y "

${APP} update

${AINST} libsdl2-dev libsdl2-2.0-0 -y;

#install sdl image  - if you want to display images
${AINST} libjpeg-dev libwebp-dev libtiff5-dev libsdl2-image-dev libsdl2-image-2.0-0 -y;

#install sdl mixer  - if you want sound
${AINST} libmikmod-dev libfishsound1-dev libsmpeg-dev liboggz2-dev libflac-dev libfluidsynth-dev libsdl2-mixer-dev libsdl2-mixer-2.0-0 -y;

#install sdl true type fonts - if you want to use text
${AINST} libfreetype6-dev libsdl2-ttf-dev libsdl2-ttf-2.0-0 -y;

${AINST} build-essential gcc-multilib g++-multilib libx11-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev libxcursor1 libasound2-dev libgtk-3-dev flex bison

${AINST} clang-tools-17

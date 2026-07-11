#!/bin/sh

setxkbmap -layout us
export LD_LIBRARY_PATH=.
./glquake.sdl -nosound -window -width 640 -height 480 $@
setxkbmap -layout fr

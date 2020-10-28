#!/usr/bin/env zsh

#
#  relocate.zsh - Relocate lib to install/extern for MacOS
#
#  Date created: May 7, 2020
#
#  Author:       Michael E. Tryby
#                US EPA - ORD/CESER
#
#  Arguments:
#    1 - Path to library
#

echo "INFO: Relocating ... $1"

if [[ -z "$1" ]]; then
    echo "ERROR: Required argument is missing"; return 1
fi

chmod 755 extern/$1:t


# Grabs current path for lib
IFS=$' '
TOKEN=($( otool -l extern/$1:t | grep LC_ID_DYLIB -A2 | grep name ))
LIB_PATH=${TOKEN[2]}


# Changes load path for runswmm
install_name_tool -change ${LIB_PATH} @rpath/$1:t bin/runswmm

# Changes load path for lib in libswmm5
install_name_tool -change ${LIB_PATH} @rpath/$1:t lib/libswmm5.dylib

# Changes id on relocated lib
install_name_tool -id @rpath/$1:t extern/$1:t

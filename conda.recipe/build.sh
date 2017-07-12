#!/bin/bash

# Make build directory and change to it
mkdir build
cd build

# Configure using the CMakeFiles
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DCMAKE_BUILD_TYPE:STRING=Release ..

# Build
make -j $CPU_COUNT

# Install
make install

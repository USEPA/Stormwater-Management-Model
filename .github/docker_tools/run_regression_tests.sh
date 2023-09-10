#!/bin/bash

#script for running regression tests in arm64 docker container

# setup environment
git config --global --add safe.directory /swmm
cd swmm
python3 -m venv venv
source venv/bin/activate
pip install aenum nrtest swmm-toolkit nrtest-swmm

# export build vars
export BUILD_HOME="build"
export PROJECT="swmm"
export PLATFORM="linux"

# run regression tests
./ci-tools/linux/make.sh
source ./ci-tools/linux/before-nrtest.sh
bash ./ci-tools/linux/run-nrtests.sh

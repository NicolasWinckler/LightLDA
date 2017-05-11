#!/bin/bash

THIRD_PARTY_PATH="/usr/local"
number_of_processes=`nproc`
rm -rf build
mkdir -p build
cd build
CMAKE_ARGS+=" -DUSE_ZMQ_PATH=$THIRD_PARTY_PATH"
CMAKE_ARGS+=" -DMPI_CXX_COMPILER=$THIRD_PARTY_PATH/bin/mpicxx"
CMAKE_ARGS+=" -DCMAKE_INSTALL_PREFIX=$THIRD_PARTY_PATH"
cmake $CMAKE_ARGS ..
make -j$number_of_processes
cd ..

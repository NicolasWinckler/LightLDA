#!/bin/bash

# build lightlda
#git clone -b multiverso-initial git@github.com:Microsoft/multiverso.git
git clone -b multiverso-initial git@github.com:NicolasWinckler/multiverso.git
MULTIVERSO_DIR="$PWD/multiverso"
THIRDPARTY_DIR="$PWD/multiverso/third_party"
CMAKE_ARGS="-DUSE_MULTIVERSO_PATH=$MULTIVERSO_DIR"
CMAKE_ARGS+=" -DUSE_ZMQ_PATH=$THIRDPARTY_DIR"
CMAKE_ARGS+=" -DMPI_CXX_COMPILER=$THIRDPARTY_DIR/bin/mpicxx"

# install lightlda dependencies, i.e. multiverso, zmq and mpi
cd multiverso
cd third_party
sh install.sh
cd ..
make -j4 all

cd ..
# install lightlda
mkdir build
cd build
cmake $CMAKE_ARGS ..
make -j4
cd ..



# build lightlda

#git clone -b multiverso-initial git@github.com:Microsoft/multiverso.git
git clone -b multiverso-initial git@github.com:NicolasWinckler/multiverso.git
cd multiverso
cd third_party
sh install.sh
cd ..
make -j4 all

cd ..
mkdir build
cd build
cmake ..
make -j4
cd ..

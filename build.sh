set -e
make clean
make -j1
cp uniLoader AIK-Linux/split_img/boot-kernel
cd AIK-Linux
./repackimg.sh
cd ..
echo ***BUILT***

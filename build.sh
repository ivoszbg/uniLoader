set -e
make clean
make x1s_defconfig
make -j1
cp uniLoader AIK-Linux/split_img/boot.img-kernel
cd AIK-Linux
./repackimg.sh
cp unsigned-new.img ../boot.img
cd ..
echo ***BUILT***

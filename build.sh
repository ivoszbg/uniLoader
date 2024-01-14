set -e
while getopts d: flag
do
	case "${flag}" in
		d) device=${OPTARG};;
	esac
done

if [ -z "$device" ]
then
	echo "Syntax:"
	echo "./build.sh -d [device]"
	exit 1
fi

if [ -f ".config" ]
then
	rm .config
fi
make clean
make "$device"_defconfig
make -j$(nproc --all)
cp uniLoader AIK-Linux/split_img/boot-kernel
cd AIK-Linux
./repackimg.sh
cd ..
echo ***BUILT***

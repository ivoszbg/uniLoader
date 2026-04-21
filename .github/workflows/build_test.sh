#!/bin/sh
#
# CI calls this script from the project root, as such we don't really
# need to care about changing the directory
#

set -e

devices=()
procs=$(nproc --all)
makeargs="CROSS_COMPILE=aarch64-linux-gnu- -j$procs"

echo "Using $procs cores for build."

# Get all available device codenames
for config in configs/*; do
    split=(${config//[\/_]/ })
    devices+=("${split[1]}")
done

# And... build.
for device in "${devices[@]}"; do
    make distclean $makeargs
    make "${device}_defconfig" $makeargs
    make $makeargs
done

echo "All done, glhf!"

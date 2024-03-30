#!/bin/bash

# Exit immediately if something fails
set -e

if [[ $(basename "$PWD") != "EmojiRunner"* ]];then
    git clone https://github.com/alex1701c/EmojiRunner
    cd EmojiRunner/
fi

mkdir -p build
cd build
krunner_version=$(krunner --version | grep -oP "(?<=krunner )\d+")
if [[ "$krunner_version" == "6" ]]; then
    echo "Building for Plasma6"
    BUILD_QT6_OPTION="-DBUILD_WITH_QT6=ON"
else
    echo "Building for Plasma5"
    BUILD_QT6_OPTION=""
fi

cmake .. -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_USE_QT_SYS_PATHS=ON $BUILD_QT6_OPTION
make -j$(nproc)
sudo make install

# KRunner needs to be restarted for the changes to be applied
if pgrep -x krunner > /dev/null
then
    kquitapp5 krunner
fi

echo "Installation finished!";

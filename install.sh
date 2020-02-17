#!/bin/bash

# Exit immediately if something fails
set -e

if [[ $(basename "$PWD") != "EmojiRunner"* ]];then
    git clone https://github.com/alex1701c/EmojiRunner
    cd EmojiRunner/
fi

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_QTPLUGINDIR=$(kf5-config --qt-plugins)  ..
make -j$(nproc)
sudo make install

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished!";

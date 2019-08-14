#!/bin/bash

# Exit immediately if something fails
set -e

if [[ $(basename "$PWD") != "EmojiRunner"* ]];then
    git clone https://github.com/alex1701c/EmojiRunner
    cd EmojiRunner/
fi
# Handle permissions for uinput module
sudo addgroup uinput
sudo adduser $(whoami) uinput
echo '"KERNEL=="uinput", GROUP="uinput", MODE="0660"' >> /etc/udev/rules.d/uinput.rules

mkdir -p build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make -j2
sudo make install

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished !";

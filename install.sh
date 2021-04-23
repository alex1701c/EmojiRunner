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

# KRunner needs to be restarted for the changes to be applied
if pgrep -x krunner > /dev/null
then
    kquitapp5 krunner
fi

# If KRunner does not get started using the shortcut we have to autostart it
krunner_version=$(krunner --version | cut -d " " -f2)
major_version=$(echo $krunner_version | cut -d "." -f -1)
minor_version=$(echo $krunner_version | cut -d "." -f2)
if [[ (("$major_version" < "5")) || (("$minor_version" < "17")) ]]
then
    kstart5 --windowclass krunner
fi

echo "Installation finished!";

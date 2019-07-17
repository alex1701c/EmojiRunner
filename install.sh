#!/bin/bash

# TODO Enable if the project is on Github
if [[ $(basename "$PWD") != "EmojiRunner"* ]];then
    #git clone https://github.com/%{USERNAME}/EmojiRunner
    #cd EmojiRunner/
    echo "Please go to the project folder"
    exit
fi

mkdir -p build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make -j2
sudo make install

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished !";

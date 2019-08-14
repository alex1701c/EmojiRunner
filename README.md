## Emoji Runner

This plugin allows you to search, copy and  paste emojis/symbols.

Additionally you can configure favourites, set the unicode version and enable/disable categories.

### Required Dependencies

Debian/Ubuntu:  
`sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext python3 python3-uinput`

openSUSE:  
`sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel
ktextwidgets-devel kservice-devel krunner-devel gettext-tools`  

Fedora:  
`sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext python3 python-uinput`  

You can also install the python library using:
`pip3 install python-uinput --user`
### Build instructions  

The easiest way to install is:  
`curl https://raw.githubusercontent.com/alex1701c/EmojiRunner/master/install.sh | bash`

Or you can do it manually:

```
git clone https://github.com/alex1701c/EmojiRunner
cd EmojiRunner
mkdir build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make
sudo make install
kquitapp5 krunner 2> /dev/null; kstart5 --windowclass krunner krunner > /dev/null 2>&1 &
```

If you want to paste the emojis directly using the shortcuts sure that your user has permissions to access the uinput module:
- Create uinput group `sudo addgroup uinput`
- Add yourself to this group `sudo adduser $(whoami) uinput`
- Create udev rule by writing `KERNEL=="uinput", GROUP="uinput", MODE="0660"` to `/etc/udev/rules.d/uinput.rules`  

The emoji.json file is a modified version from https://github.com/github/gemoji  

### Enable Shortcuts
This project has a EmojiRunnerCommands.khotkeys file.
This file contains two shortcuts (of course you can change them): 
- Ctrl+Alt+Space  Shows runner and favourites, if one emoji is selected it gets pasted to the current text input (demonstrated in gif)
- Ctrl+Alt+Shift+Space Same as above except the favourites are not shown

You can import them in the System Settings module "Custom Shortcuts" by clicking on edit and then import.  
 
The feature to paste the emojis to the can be disabled in the config dialog.  

![Search for emoji and paste](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/paste_emoji.gif)
### Screenshots:

#### Search for an emoji by name
This global search can be enabled/disabled in the config dialog  
![Search for an emoji by name](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/global_search.png)

#### Show favourites
They are configured in the config dialog  
![Show favourites](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/favourites.png)

#### Search for emojis
This way you can search for emojis without search results from other plugins  
![Search for emojis](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/search_with_prefix.png)  

#### Configure settings
You can search for emojis, set the unicode level (later versions can not be displayed) and you can disable categories  
![Search for emojis](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/config.png)  
*The sorting of the favourites inside Krunner does not work as intended, (they are not always sorted by relevance). Help is appreciated!*  

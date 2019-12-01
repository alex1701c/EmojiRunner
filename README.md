## Emoji Runner

This plugin allows you to search, copy and  paste emojis/symbols.

Additionally you can configure favourites, set the unicode version, enable/disable categories and add custom emojis.  
If you don't have colorful emojis please try the solution from https://github.com/alex1701c/EmojiRunner/issues/1/ 

### Required Dependencies

Debian/Ubuntu:  
`sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext xdotool`

openSUSE:  
`sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel
ktextwidgets-devel kservice-devel krunner-devel gettext-tools xdotool kconfigwidgets-devel`  

Fedora:  
`sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext xdotool`  

### Build instructions  

The easiest way to install is:  
`curl https://raw.githubusercontent.com/alex1701c/EmojiRunner/master/install.sh | bash`  
Or you can install it just for your user without admin privileges:  
`curl https://raw.githubusercontent.com/alex1701c/EmojiRunner/master/install-user.sh | bash`

Or you can do it manually:

```
git clone https://github.com/alex1701c/EmojiRunner
cd EmojiRunner
mkdir build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` -DCMAKE_BUILD_TYPE=Release  ..
make
sudo make install
kquitapp5 krunner 2> /dev/null; kstart5 --windowclass krunner krunner > /dev/null 2>&1 &
```

The emoji.json file is a modified version from https://github.com/github/gemoji  

### Enable Shortcuts
This project has a EmojiRunnerCommands.khotkeys file.
This file contains two shortcuts (of course you can change them): 
- Ctrl+Alt+Space  Shows only EmojiRunner and favourites,
if one emoji is selected it gets pasted to the current text input (demonstrated in gif).
For this the Runner emits the Ctrl+V key combination using xdotool
- Ctrl+Alt+Shift+Space Same as above except the favourites are not shown  

These shortcuts start Krunner using qdbus. For example the command to launch the favourites is:  
`qdbus org.kde.krunner /App org.kde.krunner.App.querySingleRunner "emojirunner" "emoji "`

You can import them in the System Settings module "Custom Shortcuts" by clicking on edit and then import.  
 
The feature to paste the emojis can be disabled in the config dialog.  

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
You can search for emojis, set the unicode level (later versions can not be displayed) and you can enable/disable categories  
![Search for emojis](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/config.png)  

#### Add a custom emoji
You must provide a name and and emoji, the description and tags are optional  
![Add a custom emoji](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/enter_custom_emoji.png)  
This gets saved in the ~/.local/share/emojirunner/customemojis.json file.  
In this file you can also override the existing emojis, but you have to do that manually.  
![Custom emojis file](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/custom_emojis_file.png)    

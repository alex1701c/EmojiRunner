This project is in development !

This plugin allows you to search and copy emojis/symbols to the clipboard.
If has the following categories:  
Smileys & Emotion, Objects, Food & Drink, People & Body, Animals & Nature, Symbols, Travel & Places and Flags.

In the future a config dialog which lets you configure favourites, categories etc. will be added.
Furthermore a search by tag name will bge implemented.

### Required Dependencies

Debian/Ubuntu:  
`sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext`

openSUSE:  
`sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel
ktextwidgets-devel kservice-devel krunner-devel gettext-tools`  

Fedora:  
`sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext`  

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
```

Restart krunner to load the runner (in a terminal type: kquitapp5 krunner;kstart5 krunner )

After this you should see your runner in the system settings:

systemsettings5 (Head to "Search")

You can also launch KRunner via Alt-F2 and you will find your runner.

The emoji.json file is a modified version from https://github.com/github/gemoji

### Screenshots:

#### Search for an emoji by name
![Search for an emoji by name](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/global_search.png)

#### Show favourites
They will be made configurable in a dialog  
![Show favourites](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/favourites.png)

#### Search for emojis
This way you can search for emojis without search results from other plugins
![Search for emojis](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/search_with_prefix.png)

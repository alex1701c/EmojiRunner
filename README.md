## Emoji Runner

- [Search emojis](#user-content-search-for-an-emoji-by-name) and copy & paste them
- Configure [favourite emojis](#show-favourites)
- Set unicode version
- Enable/disable categories
- [Add custom emojis](#add-a-custom-emoji)

You can build this package from source or use the deb/rpm packages from [the releases page](https://github.com/alex1701c/EmojiRunner/releases).

### Required Dependencies

**Note**: If the `xdo.h` file is not at compile time available the plugin will use the `xdotool` program to paste emojis, 
but having the library installed is recommended.

<details>
<summary><b>Debian/Ubuntu</b></summary>

```bash
sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext libxdo-dev libkf5kcmutils-dev
```

</details>

<details>
<summary><b>OpenSUSE</b></summary>

```bash
sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools xdotool-devel kconfigwidgets-devel kcmutils-devel
```

</details>

<details>
<summary><b>Fedora</b></summary>

```bash
sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext xdotool kf5-kcmutils-devel
```

</details>

<details>
<summary><b>Arch (Manjaro)</b></summary>

```bash
sudo pacman -S cmake extra-cmake-modules xdotool kcmutils
```
*This xdotool package includes the xdo.h file*

</details>

### Build instructions  

#### Option A: Easy oneliner method
 
```bash
curl https://raw.githubusercontent.com/alex1701c/EmojiRunner/master/install.sh | bash
```

#### Option B: Get it from the [AUR](https://aur.archlinux.org/packages/plasma5-runners-emoji/)

#### Option C: Manual installation method

```bash
git clone https://github.com/alex1701c/EmojiRunner
cd EmojiRunner
bash install.sh
```

**Info:** The `emoji.json` file is a modified version from [gemoji](https://github.com/github/gemoji).

### Enable Shortcuts
This project has a EmojiRunnerCommands.khotkeys file.
This file contains two shortcuts (of course you can change them): 
- `Ctrl` + `Alt` + `Space` Shows only EmojiRunner and favourites,
if one emoji is selected it gets pasted to the current text input (demonstrated in gif).
For this the Runner emits the `Ctrl` + `V` key combination using xdotool
- `Ctrl` + `Alt` + `Shift` + `Space` Same as above except the favourites are not shown  

These shortcuts start Krunner using qdbus. For example the command to launch the favourites is:  
`qdbus org.kde.krunner /App org.kde.krunner.App.querySingleRunner "emojirunner" "emoji "`

You can import them in the System Settings module "Custom Shortcuts" by clicking on edit and then import.  
 
The feature to paste the emojis can be disabled in the config dialog.  

![Search for emoji and paste](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/paste_emoji.gif)

### Screenshots:

#### Search for an emoji by name
This global search can be enabled/disabled in the config dialog.  
Additionally this screenshot shows the paste action, which is a new feature.  
![Search for an emoji by name](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/global_search_with_actions.png)

#### Show favourites
They are configured in the config dialog  
![Show favourites](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/favourites.png)

#### Search for emojis
This way you can search for emojis without search results from other plugins  
![Search for emojis](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/search_with_prefix.png)  

#### Configure settings
You can search for emojis, set the unicode level (later versions can not be displayed) and you can enable/disable categories.
Additionally you can sort the favourites using drag and drop  
![Search for emojis](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/config.png)  

#### Add a custom emoji
You must provide a name and and emoji, the description and tags are optional  
![Add a custom emoji](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/enter_custom_emoji.png)  

This gets saved in the ~/.local/share/emojirunner/customemojis.json file.  
![Custom emojis file](https://raw.githubusercontent.com/alex1701c/Screenshots/master/EmojiRunner/custom_emojis_file.png)    

# Issues 
- If you don't have colorful emojis please try [this solution](https://github.com/alex1701c/EmojiRunner/issues/1/).
- If the emojis do not get pasted try to increase the timeout for the pasting action by adding `pasteTimeout=timeout_in_ms` to the `[Config]` group in `~/.config/krunnerplugins/emojirunnerrc`.

If you have any other issues please let me know 😄.
 

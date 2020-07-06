#ifndef EMOJIRUNNER_CONFIG_H
#define EMOJIRUNNER_CONFIG_H

namespace {
struct Config {
    // General config keys
    constexpr static const auto ConfigFileName = "emojirunnerrc";
    constexpr static const auto RelativeConfigFolder = "/.config/krunnerplugins/";
    const static QString ConfigFilePath;
    const static QString GlobalEmojiFilePath;
    const static QString LocalEmojiFilePath;
    const static QString CustomEmojiFilePath;
    constexpr static const auto RootGroup = "Config";
    constexpr static const auto GlobalSearch = "globalSearch";
    constexpr static const auto PasteAction = "pasteAction";
    constexpr static const auto PasteTimeout = "pasteTimeout";
    constexpr static const auto SearchByTags = "searchByTags";
    constexpr static const auto SearchByDescription = "searchByDescription";
    constexpr static const auto SingleRunnerModePaste = "singleRunnerModePaste";
    constexpr static const auto Favourites = "favourites";
    constexpr static const auto DefaultFavourites = "7;1;37;14;18;154;77;36;10;111;59;23;33;87;167;168";
    // Unicode/Ios version data
    constexpr static const auto UnicodeVersion = "unicodeVersion";
    constexpr static const auto DefaultUnicodeVersion = "11.0";
    constexpr static const auto UnicodeVersionChoices = "3.0,3.2,4.0,4.1,5.1,5.2,6.0,6.1,7.0,8.0,9.0,11.0,12.0";
    constexpr static const auto IosVersion = "iosVersion";
    constexpr static const auto DefaultIosVersion = "13.0";
    constexpr static const auto IosVersionChoices = "6.0,8.3,9.0,9.1,10.0,10.2,12.1,13.0";
    // Category names
    constexpr static const auto FavouritesCategory = "Favourites";
    constexpr static const auto CustomCategory = "Custom";
    constexpr static const auto SmileysEmotionsCategory = "Smileys & Emotion";
    constexpr static const auto DisabledCategories = "disabledCategories";
};

// Keys of the emoji json object
struct JSONEmoji {
    constexpr static const auto Id = "id";
    constexpr static const auto Name = "name";
    constexpr static const auto Emoji = "emoji";
    constexpr static const auto Description = "description";
    constexpr static const auto UnicodeVersion = "unicode_version";
    constexpr static const auto IosVersion = "ios_version";
    constexpr static const auto Tags = "tags";
};

const QString Config::ConfigFilePath = QDir::homePath() + Config::RelativeConfigFolder + Config::ConfigFileName;
const QString Config::CustomEmojiFilePath = QDir::homePath() + "/.local/share/emojirunner/customemojis.json";
const QString Config::LocalEmojiFilePath = QDir::homePath() + "/.local/share/emojirunner/emojis.json";
const QString Config::GlobalEmojiFilePath = QStringLiteral("/usr/share/emojirunner/emojis.json");
}
#endif //EMOJIRUNNER_CONFIG_H

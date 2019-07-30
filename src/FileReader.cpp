#include <QtCore>
#include <QMap>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include "FileReader.h"

QList<EmojiCategory> FileReader::readJSONFile(bool getAllEmojis) {
    QList<EmojiCategory> categories;
    KConfigGroup config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");

    QFile file(QDir::homePath() + "/.config/emojis.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString content = file.readAll();
        auto emojiObject = QJsonDocument::fromJson(content.toLocal8Bit()).object();
        if (getAllEmojis) return parseAllEmojis(emojiObject, categories, config);
        return parseEnabledEmojis(emojiObject, categories, config);
    }
    return categories;
}

QList<EmojiCategory>
FileReader::parseEnabledEmojis(QJsonObject &emojiObject, QList<EmojiCategory> &categories, const KConfigGroup &config) {

    EmojiCategory favourites("Favourites");
    QStringList disabledCategories = config.readEntry("disabledCategories").split(";", QString::SplitBehavior::SkipEmptyParts);
    QList<int> favouriteIds;
    for (const auto &favouriteId:config.readEntry("favourites", "7;1;37;14;18;154;77;36;10;111;59;23;33;87;167;168")
            .split(";", QString::SplitBehavior::SkipEmptyParts))
        favouriteIds.append(favouriteId.toInt());
    float configUnicodeVersion = config.readEntry("unicodeVersion", "11").toFloat();
    float configIosVersion = config.readEntry("iosVersion", "13").toFloat();

    for (const auto &categoryKey:emojiObject.keys()) {
        EmojiCategory category(categoryKey);
        for (const auto &jsonObj:emojiObject[categoryKey].toArray()) {
            auto obj = jsonObj.toObject();
            if (obj.isEmpty()) continue;
            auto emoji = Emoji::fromJSON(obj, categoryKey);
            if (favouriteIds.contains(emoji.id)) {
                emoji.favourite = 25 - favouriteIds.indexOf(emoji.id);
                favourites.emojis.insert(emoji.name, emoji);
            }
            // Insert if favourite or ( the category is not disabled and versions match)
            if (emoji.favourite != 0 ||
                (!disabledCategories.contains(categoryKey) && emoji.matchesVersions(configUnicodeVersion, configIosVersion))) {
                category.emojis.insert(emoji.name, emoji);
            }
        }
        categories.append(category);
    }
    categories.append(favourites);
    return categories;
}

QList<EmojiCategory>
FileReader::parseAllEmojis(QJsonObject &emojiObject, QList<EmojiCategory> &categories, const KConfigGroup &config) {

    EmojiCategory favourites("Favourites");
    QList<int> favouriteIds;
    for (const auto &favouriteId:config.readEntry("favourites", "7;1;37;14;18;154;77;36;10;111;59;23;33;87;167;168")
            .split(";", QString::SplitBehavior::SkipEmptyParts))
        favouriteIds.append(favouriteId.toInt());

    for (const auto &categoryKey:emojiObject.keys()) {
        EmojiCategory category(categoryKey);
        for (const auto &jsonObj:emojiObject[categoryKey].toArray()) {
            auto obj = jsonObj.toObject();
            if (obj.isEmpty()) continue;
            auto emoji = Emoji::fromJSON(obj, categoryKey);
            if (favouriteIds.contains(emoji.id)) {
                emoji.favourite = 21 - favouriteIds.indexOf(emoji.id);
                favourites.emojis.insert(emoji.name, emoji);
            }
            category.emojis.insert(emoji.name, emoji);
        }
        categories.append(category);
    }
    categories.append(favourites);
    return categories;
}

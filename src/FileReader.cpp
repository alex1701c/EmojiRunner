#include <QtCore>
#include <QMap>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include "FileReader.h"

/**
 * Initialize reusable variables
 */
FileReader::FileReader(const KConfigGroup &config) {
    for (const auto &idString: config.readEntry("favourites", "7;1;37;14;18;154;77;36;10;111;59;23;33;87;167;168")
            .split(";", QString::SplitBehavior::SkipEmptyParts)) {
        favouriteIds.append(idString.toInt());
    }

    configUnicodeVersion = config.readEntry("unicodeVersion", "11").toFloat();
    configIosVersion = config.readEntry("iosVersion", "13").toFloat();
    disabledCategories = config.readEntry("disabledCategories").split(";", QString::SplitBehavior::SkipEmptyParts);
}

/**
 * Reads the emojis from the different files
 * Rewrite of readJSONFile function but more maintainable and ~20% faster
 *
 * @param getAllEmojis
 * @return
 */
QList<EmojiCategory> FileReader::getEmojiCategories(bool getAllEmojis) {
    // Emojis for user level install
    QFile globalFile("/usr/share/emojirunner/emojis.json");
    QFile localFile(QDir::homePath() + "/.local/share/emojirunner/emojis.json");
    QMap<QString, EmojiCategory> preconfiguredEmojis;
    if (globalFile.exists() && globalFile.open(QIODevice::ReadOnly)) {
        preconfiguredEmojis = parseEmojiFile(getAllEmojis, globalFile);
        globalFile.close();
    } else if (localFile.exists() && localFile.open(QIODevice::ReadOnly)) {
        preconfiguredEmojis = parseEmojiFile(getAllEmojis, localFile);
        localFile.close();
    }

    // Read custom emojis
    QFile customEmojis(QDir::homePath() + "/.local/share/emojirunner/customemojis.json");
    if (customEmojis.exists() && customEmojis.open(QFile::ReadOnly)) {
        auto customEmojiMap = parseEmojiFile(getAllEmojis, customEmojis);
        customEmojis.close();
        // Combine the two maps
        for (auto &category:customEmojiMap.values()) {
            if (preconfiguredEmojis.contains(category.name)) {
                const EmojiCategory existingCategory = preconfiguredEmojis.value(category.name);
                for (const auto &emoji:existingCategory.emojis.values()) {
                    category.emojis.insert(emoji.name, emoji);
                }
            }
            preconfiguredEmojis.insert(category.name, category);

        }
    }

    return preconfiguredEmojis.values();
}

QMap<QString, EmojiCategory> FileReader::parseEmojiFile(bool getAllEmojis, QFile &emojiJSONFile) {
    // Initialize emojis object and check exit conditions
    const QJsonDocument emojis = QJsonDocument::fromJson(emojiJSONFile.readAll());
    QMap<QString, EmojiCategory> categories;
    if (!emojis.isObject() || emojis.object().keys().isEmpty()) return categories;

    // Initialize config variables
    auto favourites = EmojiCategory("Favourites");

    // Read categories and items from object
    const QJsonObject emojiRootObject = emojis.object();
    for (const auto &key:emojiRootObject.keys()) {
        // Get items array and check continue conditions
        const QJsonValue value = emojiRootObject.value(key);
        if (!value.isArray()) continue;
        const QJsonArray items = value.toArray();
        if (items.isEmpty()) continue;

        // Create a new category/find the existing one and add the emojis to it
        EmojiCategory category(key);
        // Even if the category is disabled the favourites get added
        const bool categoryDisabled = disabledCategories.contains(category.name);

        // Add emojis to category
        for (const auto &item:items) {
            if (!item.isObject()) continue;
            Emoji customEmoji = Emoji::fromJSON(item.toObject(), key);
            // Add emoji to the favourites list
            const int favouritesIdx = favouriteIds.indexOf(customEmoji.id);
            // Favourites have extra list for overview
            if (favouritesIdx != -1) {
                customEmoji.favourite = 21 - favouritesIdx;
                favourites.emojis.insert(customEmoji.name, customEmoji);
            }
            // Add emoji to category
            if (getAllEmojis || customEmoji.favourite != 0 ||
                (!categoryDisabled && customEmoji.matchesVersions(configUnicodeVersion, configIosVersion))) {
                category.emojis.insert(customEmoji.name, customEmoji);
            }
        }
        if (!category.emojis.isEmpty()) categories.insert(category.name, category);
    }
    categories.insert(favourites.name, favourites);
    return categories;
}



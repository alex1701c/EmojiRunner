#include <QtCore>
#include <QMap>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include "FileReader.h"
#include <core/Config.h>

FileReader::FileReader(const KConfigGroup &config) {
    for (const auto &idString: config.readEntry(Config::Favourites, Config::DefaultFavourites)
            .split(";", QString::SplitBehavior::SkipEmptyParts)) {
        favouriteIds.append(idString.toInt());
    }

    configUnicodeVersion = config.readEntry(Config::UnicodeVersion, QVariant(Config::DefaultUnicodeVersion).toFloat());
    configIosVersion = config.readEntry(Config::IosVersion, QVariant(Config::DefaultIosVersion).toFloat());
    disabledCategories = config.readEntry(Config::DisabledCategories).split(";", QString::SplitBehavior::SkipEmptyParts);
}

QList<EmojiCategory> FileReader::getEmojiCategories(bool getAllEmojis) const {
    // Emojis for user level install
    QFile globalFile(Config::GlobalEmojiFilePath);
    QFile localFile(Config::LocalEmojiFilePath);
    QMap<QString, EmojiCategory> preconfiguredEmojis;
    if (globalFile.exists() && globalFile.open(QIODevice::ReadOnly)) {
        preconfiguredEmojis = parseEmojiFile(getAllEmojis, globalFile);
        globalFile.close();
    } else if (localFile.exists() && localFile.open(QIODevice::ReadOnly)) {
        preconfiguredEmojis = parseEmojiFile(getAllEmojis, localFile);
        localFile.close();
    }

    // Read custom emojis
    QFile customEmojis(Config::CustomEmojiFilePath);
    if (customEmojis.exists() && customEmojis.open(QFile::ReadOnly)) {
        auto customEmojiMap = parseEmojiFile(getAllEmojis, customEmojis);
        customEmojis.close();
        // Combine the two maps
        for (auto &category:customEmojiMap.values()) {
            if (preconfiguredEmojis.contains(category.name)) {
                const EmojiCategory existingCategory = preconfiguredEmojis.value(category.name);
                for (auto *emoji:existingCategory.emojis) {
                    category.emojis.append(emoji);
                }
            }
            preconfiguredEmojis.insert(category.name, category);
        }
    }

    return preconfiguredEmojis.values();
}

QMap<QString, EmojiCategory> FileReader::parseEmojiFile(bool getAllEmojis, QFile &emojiJSONFile) const {
    // Initialize emojis object and check exit conditions
    const QJsonDocument emojis = QJsonDocument::fromJson(emojiJSONFile.readAll());
    QMap<QString, EmojiCategory> categories;
    if (!emojis.isObject() || emojis.object().keys().isEmpty()) return categories;

    // Initialize config variables
    auto favourites = EmojiCategory(Config::FavouritesCategory);

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
            Emoji *newEmoji = Emoji::fromJSON(item.toObject(), key);
            // Add emoji to the favourites list
            const int favouritesIdx = favouriteIds.indexOf(newEmoji->id);
            // Favourites have extra list for overview
            if (favouritesIdx != -1) {
                newEmoji->favourite = 21 - favouritesIdx;
                favourites.emojis.append(newEmoji);
            }
            // Add emoji to category
            if (getAllEmojis || newEmoji->favourite != 0 ||
                (!categoryDisabled && newEmoji->matchesVersions(configUnicodeVersion, configIosVersion))) {
                category.emojis.append(newEmoji);
            }
        }
        if (!category.emojis.isEmpty()) categories.insert(category.name, category);
    }
    categories.insert(favourites.name, favourites);
    return categories;
}



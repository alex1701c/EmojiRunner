#include "FileReader.h"
#include <KConfigCore/KConfigGroup>
#include <KConfigCore/KSharedConfig>
#include <QMap>
#include <QtCore>
#include <core/Config.h>

FileReader::FileReader(const KConfigGroup &config)
{
    const auto idStringList = config.readEntry(Config::Favourites, Config::DefaultFavourites).split(';', Qt::SkipEmptyParts);
    for (const auto &idString : idStringList) {
        favouriteIds.append(idString.toInt());
    }

    configUnicodeVersion = config.readEntry(Config::UnicodeVersion, QVariant(Config::DefaultUnicodeVersion).toFloat());
    configIosVersion = config.readEntry(Config::IosVersion, QVariant(Config::DefaultIosVersion).toFloat());
    disabledCategories = config.readEntry(Config::DisabledCategories).split(';', Qt::SkipEmptyParts);
}

QList<EmojiCategory> FileReader::getEmojiCategories(bool getAllEmojis) const
{
    // Emojis for user level install
    QString emojisFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, Config::SharedEmojiFileName, QStandardPaths::LocateFile);
    QFile emojisFile(emojisFilePath);
    QMap<QString, EmojiCategory> preconfiguredEmojis;
    if (emojisFile.exists() && emojisFile.open(QIODevice::ReadOnly)) {
        preconfiguredEmojis = parseEmojiFile(getAllEmojis, emojisFile);
    }

    // Read custom emojis
    QFile customEmojis(Config::CustomEmojiFilePath);
    if (customEmojis.exists() && customEmojis.open(QFile::ReadOnly)) {
        auto customEmojiMap = parseEmojiFile(getAllEmojis, customEmojis);
        // Combine the two maps
        auto customEmojiIT = customEmojiMap.constBegin();
        while (customEmojiIT != customEmojiMap.constEnd()) {
            auto category = customEmojiIT.value();
            ++customEmojiIT;
            if (preconfiguredEmojis.contains(category.name)) {
                const EmojiCategory existingCategory = preconfiguredEmojis.value(category.name);
                for (const auto &emoji : existingCategory.emojis) {
                    category.emojis.append(emoji);
                }
            }
            preconfiguredEmojis.insert(category.name, category);
        }
    }

    return preconfiguredEmojis.values();
}

QMap<QString, EmojiCategory> FileReader::parseEmojiFile(bool getAllEmojis, QFile &emojiJSONFile) const
{
    // Initialize emojis object and check exit conditions
    const QJsonDocument emojis = QJsonDocument::fromJson(emojiJSONFile.readAll());
    QMap<QString, EmojiCategory> categories;
    if (!emojis.isObject())
        return categories;

    // Initialize config variables
    auto favourites = EmojiCategory(Config::FavouritesCategory);

    // Read categories and items from object
    const QJsonObject emojiRootObject = emojis.object();
    auto emojiRootIT = emojiRootObject.constBegin();
    while (emojiRootIT != emojiRootObject.constEnd()) {
        // Get items array and check continue conditions
        const QString key = emojiRootIT.key();
        const QJsonValue value = emojiRootIT.value();
        ++emojiRootIT;
        if (!value.isArray())
            continue;
        const QJsonArray items = value.toArray();
        if (items.isEmpty())
            continue;

        // Create a new category/find the existing one and add the emojis to it
        EmojiCategory category(key);
        // Even if the category is disabled the favourites get added
        const bool categoryDisabled = disabledCategories.contains(category.name);

        // Add emojis to category
        for (const auto &item : items) {
            if (!item.isObject())
                continue;
            Emoji newEmoji = Emoji::fromJSON(item.toObject(), key);
            // Add emoji to the favourites list
            const int favouritesIdx = favouriteIds.indexOf(newEmoji.id);
            // Favourites have extra list for overview
            if (favouritesIdx != -1) {
                newEmoji.favourite = 21 - favouritesIdx;
                favourites.emojis.append(newEmoji);
            }
            // Add emoji to category
            if (getAllEmojis || newEmoji.favourite != 0 || (!categoryDisabled && newEmoji.matchesVersions(configUnicodeVersion, configIosVersion))) {
                category.emojis.append(newEmoji);
            }
        }
        if (!category.emojis.isEmpty())
            categories.insert(category.name, category);
    }
    categories.insert(favourites.name, favourites);
    return categories;
}

#include <QtCore>
#include <QMap>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include "FileReader.h"

/**
 * Open emojis.json file and call parse method to filter the emojis
 * @param getAllEmojis
 */
QList<EmojiCategory> FileReader::readJSONFile(bool getAllEmojis) {
    QList<EmojiCategory> categories;
    KConfigGroup config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");

    QFile file("/usr/share/emojirunner/emojis.json");
    QList<int> favouriteIds;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString content = file.readAll();
        file.close();
        for (const auto &favouriteId:config.readEntry("favourites", "7;1;37;14;18;154;77;36;10;111;59;23;33;87;167;168")
                .split(";", QString::SplitBehavior::SkipEmptyParts))
            favouriteIds.append(favouriteId.toInt());

        auto emojiObject = QJsonDocument::fromJson(content.toLocal8Bit()).object();
        if (getAllEmojis) categories = parseAllEmojis(emojiObject, categories, favouriteIds);
        else categories = parseEnabledEmojis(emojiObject, categories, config, favouriteIds);

    }
    // Adds the custom emojis to the categories
    return parseCustomEmojis(categories, favouriteIds, getAllEmojis, config);
}

/**
 * Read emojis from object and insert the in the corresponding category if they are enabled
 * This method is used to get the emoji list for the runner
 * @param emojiObject
 * @param categories
 * @param config
 * @param favouriteIds
 */
QList<EmojiCategory>
FileReader::parseEnabledEmojis(const QJsonObject &emojiObject, QList<EmojiCategory> &categories,
                               const KConfigGroup &config, const QList<int> &favouriteIds) {
    EmojiCategory favourites("Favourites");
    QStringList disabledCategories = config.readEntry("disabledCategories").split(";", QString::SplitBehavior::SkipEmptyParts);
    float configUnicodeVersion = config.readEntry("unicodeVersion", "11").toFloat();
    float configIosVersion = config.readEntry("iosVersion", "13").toFloat();

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

/**
 * Read all emojis from object and insert the in the corresponding category
 * @param emojiObject
 * @param categories
 * @param favouriteIds
 */
QList<EmojiCategory>
FileReader::parseAllEmojis(QJsonObject &emojiObject, QList<EmojiCategory> &categories, const QList<int> &favouriteIds) {
    EmojiCategory favourites("Favourites");
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

/*
 * Adds the custom emojis from the ~/.local/share/emojirunner/customemojis.json file to the categories
 * TODO Add GUI for adding new emojis
 * @param favouriteIds
 * @param categories
 */
QList<EmojiCategory> FileReader::parseCustomEmojis(QList<EmojiCategory> &categories, const QList<int> &favouriteIds, bool getAllEmojis,
                                                   const KConfigGroup &config) {
    // Initialize emojis object and check exit conditions
    QFile customEmojis(QDir::homePath() + "/.local/share/emojirunner/customemojis.json");
    if (!customEmojis.exists() || !customEmojis.open(QFile::ReadOnly)) return categories;
    const QString content = customEmojis.readAll();
    customEmojis.close();
    QJsonDocument emojis = QJsonDocument::fromJson(content.toLocal8Bit());
    if (!emojis.isObject()) return categories;
    if (emojis.object().keys().isEmpty()) return categories;

    // Initialize config variables
    EmojiCategory favourites = categories.last();
    QStringList disabledCategories;
    float configUnicodeVersion = config.readEntry("unicodeVersion", "11").toFloat();
    float configIosVersion = config.readEntry("iosVersion", "13").toFloat();
    if (!getAllEmojis) {
        disabledCategories = config.readEntry("disabledCategories").split(";", QString::SplitBehavior::SkipEmptyParts);
    }

    // Read categories and items from object
    QJsonObject emojiRootObject = emojis.object();
    for (const auto &key:emojiRootObject.keys()) {
        // Get items array and check continue conditions
        QJsonValue value = emojiRootObject.value(key);
        if (!value.isArray()) continue;
        QJsonArray items = value.toArray();
        if (items.isEmpty()) continue;

        // Create a new category/find the existing one and add the emojis to it
        EmojiCategory category(key);
        int categoryCount = categories.count();
        int categoryIdx = -1;
        for (int i = 0; i < categoryCount; ++i) {
            if (categories.at(i).name == category.name) {
                category = categories.at(i);
                // Write index and cancel loop
                categoryIdx = i;
                categoryCount = -1;
            }
        }
        const bool categoryDisabled = disabledCategories.contains(category.name);

        // Add emojis to category
        for (const auto &item:items) {
            if (!item.isObject()) continue;
            Emoji customEmoji = Emoji::fromJSON(item.toObject(), key);
            // Add emoji to the favourites list
            if (favouriteIds.contains(customEmoji.id)) {
                customEmoji.favourite = 21 - favouriteIds.indexOf(customEmoji.id);
                favourites.emojis.insert(customEmoji.name, customEmoji);
            }
            if (getAllEmojis || customEmoji.favourite != 0 ||
                (!categoryDisabled && customEmoji.matchesVersions(configUnicodeVersion, configIosVersion))) {
                category.emojis.insert(customEmoji.name, customEmoji);
            }
        }
        if (categoryCount == -1) {
            // Replace values, because they are const when they are in the list
            categories.replace(categoryIdx, category);
        } else {
            // Add new category, but favourites should be at the end
            categories.prepend(category);
        }
    }
    categories.replace(categories.count() - 1, favourites);
    return categories;
}


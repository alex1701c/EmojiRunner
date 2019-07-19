#include <QtCore>
#include <QMap>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include "FileReader.h"

QList<EmojiCategory> FileReader::readJSONFile() {
    QList<EmojiCategory> categories;
    EmojiCategory favourites("Favourites");
    KConfigGroup config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");
    QList<int> favouriteIds;
    for (const auto &favouriteId:config.readEntry("favourites", "7;1;37;14;18;154;77;36;10;111;59;23;33;87;167;168")
            .split(";", QString::SplitBehavior::SkipEmptyParts))
        favouriteIds.append(favouriteId.toInt());

    QFile file(QDir::homePath() + "/.config/emojis.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString content = file.readAll();
        auto emojiObject = QJsonDocument::fromJson(content.toLocal8Bit()).object();
        for (const auto &categoryKey:emojiObject.keys()) {
            EmojiCategory category(categoryKey);
            for (const auto &jsonObj:emojiObject[categoryKey].toArray()) {
                auto obj = jsonObj.toObject();
                if (obj.isEmpty()) continue;
                Emoji emoji;
                emoji.id = obj.value("id").toInt();
                emoji.name = obj.value("name").toString();
                emoji.emoji = obj.value("emoji").toString();
                emoji.category = categoryKey;
                for (const auto &tag:obj.value("tags").toArray()) emoji.tags.append(tag.toString());
                emoji.description = obj.value("description").toString();
                emoji.unicodeVersion = obj.value("unicode_version").toString().toFloat();
                emoji.iosVersion = obj.value("ios_version").toString().toFloat();
                if (favouriteIds.contains(emoji.id)) {
                    emoji.favourite = 21 - favouriteIds.indexOf(emoji.id);
                    favourites.emojis.insert(emoji.name, emoji);
                }
                category.emojis.insert(emoji.name, emoji);
            }
            categories.append(category);
        }
    }
    categories.append(favourites);
    return categories;
}

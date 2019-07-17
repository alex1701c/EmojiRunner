#include <QtCore>
#include <QMap>
#include "FileReader.h"

QMap<QString, EmojiCategory> FileReader::readJSONFile() {
    QMap<QString, EmojiCategory> categories;

    QFile file(QDir::homePath() + "/.config/emojis.json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString content = file.readAll();
        auto emojiObject = QJsonDocument::fromJson(content.toLocal8Bit()).object();
        for (const auto &categoryKey:emojiObject.keys()) {
            for (const auto &jsonObj:emojiObject[categoryKey].toArray()) {
                qInfo() << jsonObj.toObject().value("name").toString();
                break;
                /*
                auto obj = jsonObj.toObject();
                if (obj.isEmpty()) continue;
                Emoji emoji;
                emoji.id = obj.value("id").toInt();
                emoji.name = obj.value("name").toString();
                emoji.emoji = obj.value("emoji").toString();
                emoji.category = obj.value("category").toString();
                for (const auto &tag:obj.value("tags").toArray()) {
                    emoji.tags.append(tag.toString());
                }
                emoji.description = obj.value("description").toString();
                emoji.unicodeVersion = obj.value("unicode_version").toString();
                emoji.iosVersion = obj.value("ios_version").toString();
                 */
            }
        }
    }

    return categories;
}

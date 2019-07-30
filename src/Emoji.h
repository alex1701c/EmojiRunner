#include <utility>

#include <utility>

#ifndef EMOJIRUNNER_EMOJI_H
#define EMOJIRUNNER_EMOJI_H

#include <QtCore>


class Emoji {
public:
    int id;
    int favourite = 0;
    QString name;
    QString emoji;
    QString description;
    QString category;
    QStringList tags;
    float unicodeVersion = 0;
    float iosVersion = 0;

    bool matchesVersions(float configUnicodeVersion, float configIosVersion) const {
        if (unicodeVersion != 0 && unicodeVersion > configUnicodeVersion) {
            return false;
        } else if (unicodeVersion == 0 && iosVersion > configIosVersion) {
            return false;
        }
        return true;
    }

        static Emoji fromJSON(const QJsonObject &obj, const QString &categoryKey) {
        Emoji emoji;
        emoji.id = obj.value("id").toInt();
        emoji.name = obj.value("name").toString();
        emoji.emoji = obj.value("emoji").toString();
        emoji.category = categoryKey;
        for (const auto &tag:obj.value("tags").toArray()) emoji.tags.append(tag.toString());
        emoji.description = obj.value("description").toString();
        emoji.unicodeVersion = obj.value("unicode_version").toString().toFloat();
        emoji.iosVersion = obj.value("ios_version").toString().toFloat();
        return emoji;
    }
};


#endif //EMOJIRUNNER_EMOJI_H

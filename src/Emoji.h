#include <utility>

#include <utility>

#ifndef EMOJIRUNNER_EMOJI_H
#define EMOJIRUNNER_EMOJI_H

#include <QtCore>


class Emoji {
public:
    int id = 0;
    int favourite = 0;
    QString name;
    QString emoji;
    QString description;
    QString category;
    QStringList tags;
    float unicodeVersion = 0;
    float iosVersion = 0;

    bool nameQueryMatches(const QString &search) const {
        return QString(this->name).remove("_").contains(search, Qt::CaseInsensitive) ||
               QString(this->name).replace("_", " ").contains(search, Qt::CaseInsensitive);
    }

    double tagsQueryMatches(const QString &search) const {
        for (const auto &tag:this->tags) {
            if (tag.contains(search, Qt::CaseInsensitive)) return (double) search.length() / (tag.length() * 8);
        }
        return -1;
    }

    double descriptionQueryMatches(const QString &search) const {
        if (this->description.contains(search, Qt::CaseInsensitive)) {
            return (double) search.length() / (this->description.length() * 8);
        }
        return -1;
    }

    bool matchesVersions(float configUnicodeVersion, float configIosVersion) const {
        if (unicodeVersion != 0 && unicodeVersion > configUnicodeVersion) return false;
        return !(unicodeVersion == 0 && iosVersion > configIosVersion);
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

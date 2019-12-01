#include <utility>

#include <utility>

#ifndef EMOJIRUNNER_EMOJI_H
#define EMOJIRUNNER_EMOJI_H

#include <QtCore>
#include <QtWidgets/QListWidgetItem>


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

    /**
     * Gets the relevance with all parameters and rules
     * @param search
     * @param tagSearch
     * @param descriptionSearch
     * @return double
     */
    double getEmojiRelevance(const QString &search, const bool tagSearch, const bool descriptionSearch) const {
        double res = getMatchTextRelevance(search, tagSearch, descriptionSearch);

        if (res == -1) return res;
        if (this->favourite != 0) res += 0.5;
        if (this->category == "Smileys & Emotion" || this->category == "Custom") res *= 2;
        return res;
    }

    /**
     * Gets the relevance of the compared texts
     * @param search
     * @param tagSearch
     * @param descriptionSearch
     * @return  double
     */
    double getMatchTextRelevance(const QString &search, const bool tagSearch, const bool descriptionSearch) const {
        if (this->name.contains(search)) {
            return (double) search.size() / (this->name.length() * 8);
        }
        if (descriptionSearch && this->description.contains(search)) {
            return (double) search.size() / (this->description.length() * 8);
        }
        if (tagSearch) {
            for (const auto &tag:this->tags) {
                if (tag.contains(search)) return (double) search.size() / (tag.length() * 8);
            }
        }
        return -1;
    }

    /**
     * If the emoji matches the unicode version or the fallback ios version
     * @param configUnicodeVersion
     * @param configIosVersion
     * @return bool
     */
    bool matchesVersions(const float &configUnicodeVersion, const float &configIosVersion) const {
        if (unicodeVersion != 0 && unicodeVersion > configUnicodeVersion) return false;
        return !(unicodeVersion == 0 && iosVersion > configIosVersion);
    }

    /**
     * Returns a QListWidgetItem with the data of the emoji
     * @return QListWidgetItem
     */
    QListWidgetItem *toListWidgetItem() const {
        auto *item = new QListWidgetItem(this->emoji + " " + this->name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(this->favourite != 0 ? Qt::Checked : Qt::Unchecked);
        item->setData(1, this->name);
        return item;
    }

    /**
     * Returns an Emoji instance based on the data from the JSON object
     * @param obj
     * @param categoryKey
     * @return Emoji
     */
    static Emoji fromJSON(const QJsonObject &obj, const QString &categoryKey) {
        Emoji emoji;
        emoji.id = obj.value("id").toInt();
        emoji.name = obj.value("name").toString().toLower();
        emoji.emoji = obj.value("emoji").toString();
        emoji.category = categoryKey;
        for (const auto &tag:obj.value("tags").toArray()) emoji.tags.append(tag.toString().toLower());
        emoji.description = obj.value("description").toString().toLower();
        emoji.unicodeVersion = obj.value("unicode_version").toString().toFloat();
        emoji.iosVersion = obj.value("ios_version").toString().toFloat();
        return emoji;
    }

    /**
     *
     * @param emojis Emojis that should be persisted
     * @param filePath File location, if empty the default customemojis.json file will be userd
     * @param category Category of emojis, by default "Custom"
     */
    static void writeToJSONFile(const QList<Emoji> &emojis, const QString &filePath = "", const QString &category = "Custom") {
        // Initialize values
        QJsonDocument doc;
        QJsonArray emojiJsonArray;
        QJsonObject rootObject;

        // Read existing file
        QFile existingEmojisFile(filePath.isEmpty() ? QDir::homePath() + "/.local/share/emojirunner/customemojis.json" : filePath);
        if (existingEmojisFile.exists() && existingEmojisFile.open(QFile::ReadOnly)) {
            doc = QJsonDocument::fromJson(existingEmojisFile.readAll());
            existingEmojisFile.close();
        }

        // If the user overrides existing emojis manually the changes are kept
        if (doc.isObject()) rootObject = doc.object();

        // Write emojis in json array
        const int customEmojiCount = emojis.count();
        for (int i = 0; i < customEmojiCount; ++i) {
            const Emoji &e = emojis.at(i);
            QJsonObject customObj;
            customObj.insert("emoji", e.emoji);
            customObj.insert("name", e.name.toLower());
            customObj.insert("id", QJsonValue(2000 + i));
            customObj.insert("tags", QJsonArray::fromStringList(e.tags));
            customObj.insert("description", e.description.toLower());
            customObj.insert("unicode_version", 1);
            customObj.insert("ios_version", 0);
            emojiJsonArray.append(QJsonValue(customObj));
        }

        // Update values
        rootObject.insert(category, emojiJsonArray);
        doc.setObject(rootObject);

        // Make sure that folder exists
        const QString configFolder = QDir::homePath() + "/.local/share/emojirunner/";
        const QDir configDir(configFolder);
        if (!configDir.exists()) configDir.mkpath(configFolder);

        // Write to file
        QFile configFile(configFolder + "customemojis.json");
        if (configFile.open(QIODevice::WriteOnly)) {
            configFile.write(doc.toJson());
            configFile.close();
        }
    }
};


#endif //EMOJIRUNNER_EMOJI_H

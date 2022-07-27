#ifndef EMOJIRUNNER_EMOJI_H
#define EMOJIRUNNER_EMOJI_H

#include <QtCore>
#include <QtWidgets/QListWidgetItem>
#include <utility>

class Emoji
{
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
    double getEmojiRelevance(const QString &search, bool tagSearch, bool descriptionSearch) const;

    /**
     * Gets the relevance of the compared texts
     * @param search
     * @param tagSearch
     * @param descriptionSearch
     * @return  double
     */
    double getMatchTextRelevance(const QString &search, bool tagSearch, bool descriptionSearch) const;

    /**
     * If the emoji matches the unicode version or the fallback ios version
     * @param configUnicodeVersion
     * @param configIosVersion
     * @return bool
     */
    bool matchesVersions(const float configUnicodeVersion, const float configIosVersion) const;

    /**
     * Returns an Emoji instance based on the data from the JSON object
     * @param obj
     * @param categoryKey
     * @return Emoji
     */
    static Emoji fromJSON(const QJsonObject &obj, const QString &categoryKey);

    /**
     * Writes emojis to given file
     * @param emojis Emojis that should be persisted
     * @param filePath File location, if empty the default customemojis.json file will be userd
     * @param category Category of emojis, by default "Custom"
     */
    static void writeToJSONFile(const QList<Emoji> &emojis, const QString &filePath = QString(), const QString &category = QStringLiteral("Custom"));
};

Q_DECLARE_METATYPE(Emoji)

#endif // EMOJIRUNNER_EMOJI_H

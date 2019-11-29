
#ifndef EMOJIRUNNER_FILEREADER_H
#define EMOJIRUNNER_FILEREADER_H


#include "EmojiCategory.h"

class FileReader {
public:
    KConfigGroup config;
    QList<int> favouriteIds;
    float configUnicodeVersion;
    float configIosVersion;
    QStringList disabledCategories;

    explicit FileReader(const KConfigGroup &config);

    QList<EmojiCategory> getEmojiCategories(bool getAllEmojis);

    QMap<QString, EmojiCategory> parseEmojiFile(bool getAllEmojis, QFile &emojiJSONFile);
};


#endif //EMOJIRUNNER_FILEREADER_H

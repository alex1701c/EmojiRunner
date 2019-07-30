
#ifndef EMOJIRUNNER_FILEREADER_H
#define EMOJIRUNNER_FILEREADER_H


#include "EmojiCategory.h"

class FileReader {
public:
    static QList<EmojiCategory> readJSONFile(bool getAllEmojis = false);

    static QList<EmojiCategory> parseAllEmojis(QJsonObject &emojiObject, QList<EmojiCategory> &categories, const KConfigGroup &config);

    static QList<EmojiCategory>
    parseEnabledEmojis(QJsonObject &emojiObject, QList<EmojiCategory> &categories, const KConfigGroup &config);
};


#endif //EMOJIRUNNER_FILEREADER_H


#ifndef EMOJIRUNNER_FILEREADER_H
#define EMOJIRUNNER_FILEREADER_H


#include "EmojiCategory.h"

class FileReader {
public:
    static QList<EmojiCategory> readJSONFile(bool getAllEmojis = false);

    static QList<EmojiCategory> parseAllEmojis(QJsonObject &emojiObject,
                                               QList<EmojiCategory> &categories,
                                               const QList<int> &favouriteIds);

    static QList<EmojiCategory> parseEnabledEmojis(const QJsonObject &emojiObject,
                                                   QList<EmojiCategory> &categories,
                                                   const KConfigGroup &config,
                                                   const QList<int> &favouriteIds);

    static QList<EmojiCategory>
    parseCustomEmojis(QList<EmojiCategory> &categories, const QList<int> &favouriteIds, bool getAllEmojis, const KConfigGroup &config);
};


#endif //EMOJIRUNNER_FILEREADER_H

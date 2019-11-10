
#ifndef EMOJIRUNNER_FILEREADER_H
#define EMOJIRUNNER_FILEREADER_H


#include "EmojiCategory.h"

class FileReader {
public:
    static QList<EmojiCategory> getEmojiCategories(bool getAllEmojis = false);

    static QMap<QString,EmojiCategory> parseEmojiFile(bool getAllEmojis ,QFile& emojiJSONFile);
};


#endif //EMOJIRUNNER_FILEREADER_H

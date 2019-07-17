
#ifndef EMOJIRUNNER_FILEREADER_H
#define EMOJIRUNNER_FILEREADER_H


#include "EmojiCategory.h"

class FileReader {
public:
    static QMap<QString,EmojiCategory> readJSONFile() ;
};


#endif //EMOJIRUNNER_FILEREADER_H

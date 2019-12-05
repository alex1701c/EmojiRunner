
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

    /**
     * Initialize reusable variables
     */
    explicit FileReader(const KConfigGroup &config);

    /**
     * Reads the emojis from the different files
     * Rewrite of readJSONFile function but more maintainable and ~20% faster
     *
     * @param getAllEmojis
     * @return
     */
    QList<EmojiCategory> getEmojiCategories(bool getAllEmojis) const;

    /**
     * Returns map of category name and emoji list from the given file
     * @param getAllEmojis
     * @param emojiJSONFile
     * @return
     */
    QMap<QString, EmojiCategory> parseEmojiFile(bool getAllEmojis, QFile &emojiJSONFile) const;
};


#endif //EMOJIRUNNER_FILEREADER_H

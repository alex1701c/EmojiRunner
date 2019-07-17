#ifndef EMOJIRUNNER_EMOJI_H
#define EMOJIRUNNER_EMOJI_H


#include <QtCore/QString>
#include <QtCore/QStringList>

class Emoji {
public:
    int id;
    int relevance;
    QString emoji;
    QString description;
    QString category;
    QStringList aliases;
    QStringList tags;
    QString unicodeVersion;
    QString iosVersion;

    static void readJSONFile();
};


#endif //EMOJIRUNNER_EMOJI_H

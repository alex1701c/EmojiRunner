#include <utility>

#include <utility>

#ifndef EMOJIRUNNER_EMOJI_H
#define EMOJIRUNNER_EMOJI_H


#include <QtCore/QString>
#include <QtCore/QStringList>

class Emoji {
public:
    int id;
    int favourite = 0;
    QString name;
    QString emoji;
    QString description;
    QString category;
    QStringList tags;
    float unicodeVersion = 0;
    float iosVersion=0;
};


#endif //EMOJIRUNNER_EMOJI_H

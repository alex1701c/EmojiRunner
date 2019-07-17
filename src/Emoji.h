#include <utility>

#include <utility>

#include <utility>

#ifndef EMOJIRUNNER_EMOJI_H
#define EMOJIRUNNER_EMOJI_H


#include <QtCore/QString>
#include <QtCore/QStringList>

class Emoji {
public:
    int id;
    int relevance;
    QString name;
    QString emoji;
    QString description;
    QString category;
    QStringList tags;
    QString unicodeVersion;
    QString iosVersion;
};


#endif //EMOJIRUNNER_EMOJI_H

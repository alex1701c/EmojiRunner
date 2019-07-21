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
    float iosVersion = 0;

    bool matchesVersions(float configUnicodeVersion, float configIosVersion) const {
        if (unicodeVersion != 0 && unicodeVersion > configUnicodeVersion) {
            return false;
        } else if (unicodeVersion == 0 && iosVersion > configIosVersion) {
            return false;
        }
        return true;
    }
};


#endif //EMOJIRUNNER_EMOJI_H

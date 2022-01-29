#include <utility>

#ifndef EMOJIRUNNER_EMOJICATEGORY_H
#define EMOJIRUNNER_EMOJICATEGORY_H

#include <QtCore/QString>
#include <QtCore/QMap>
#include "Emoji.h"

class EmojiCategory {
public:
    QString name;
    QList<Emoji> emojis;

    EmojiCategory() = default;

    explicit EmojiCategory(QString name) : name(std::move(name)) {}
};


#endif //EMOJIRUNNER_EMOJICATEGORY_H

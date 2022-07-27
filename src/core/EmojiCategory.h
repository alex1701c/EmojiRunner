#include <utility>

#ifndef EMOJIRUNNER_EMOJICATEGORY_H
#define EMOJIRUNNER_EMOJICATEGORY_H

#include "Emoji.h"
#include <QtCore/QMap>
#include <QtCore/QString>

class EmojiCategory
{
public:
    QString name;
    QList<Emoji> emojis;

    EmojiCategory() = default;

    explicit EmojiCategory(QString name)
        : name(std::move(name))
    {
    }
};

#endif // EMOJIRUNNER_EMOJICATEGORY_H

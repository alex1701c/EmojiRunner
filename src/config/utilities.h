#ifndef EMOJIRUNNER_UTILITIES_H
#define EMOJIRUNNER_UTILITIES_H

#include <QtWidgets/QListWidgetItem>
#include <core/Emoji.h>

namespace Utilities {

/**
 * Returns a QListWidgetItem with the data of the emoji
 * @return QListWidgetItem
 */
    QListWidgetItem *toListWidgetItem(const Emoji &emoji) {
        auto *item = new QListWidgetItem(emoji.emoji + " " + emoji.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(emoji.favourite != 0 ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, emoji.name);
        return item;
    }
}
#endif //EMOJIRUNNER_UTILITIES_H

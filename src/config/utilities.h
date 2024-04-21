#ifndef EMOJIRUNNER_UTILITIES_H
#define EMOJIRUNNER_UTILITIES_H

#include "core/Emoji.h"
#include <QListWidgetItem>

namespace Utilities
{
/**
 * Returns a QListWidgetItem with the data of the emoji
 * @return QListWidgetItem
 */
inline QListWidgetItem *toListWidgetItem(const Emoji &emoji)
{
    auto *item = new QListWidgetItem(emoji.emoji + " " + emoji.name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(emoji.favourite != 0 ? Qt::Checked : Qt::Unchecked);
    item->setData(Qt::UserRole, QVariant::fromValue(emoji));
    return item;
}
}

#endif // EMOJIRUNNER_UTILITIES_H

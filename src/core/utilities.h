#ifndef EMOJIRUNNER_UTILITIES_H
#define EMOJIRUNNER_UTILITIES_H

#include "Config.h"
#include "Emoji.h"
#include <QListWidgetItem>

namespace Utilities
{
/**
 * Returns a QListWidgetItem with the data of the emoji
 * @return QListWidgetItem
 */
QListWidgetItem *toListWidgetItem(const Emoji &emoji)
{
    auto *item = new QListWidgetItem(emoji.emoji + " " + emoji.name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(emoji.favourite != 0 ? Qt::Checked : Qt::Unchecked);
    item->setData(Qt::UserRole, QVariant::fromValue(emoji));
    return item;
}
}

/**
 * Create the config file and parent folder if they do not exist already
 */
void createConfigFile()
{
    const QString configFolder = QDir::homePath() + Config::RelativeConfigFolder;
    const QDir configDir(configFolder);
    if (!configDir.exists())
        configDir.mkpath(configFolder);
    // Create file
    QFile configFile(Config::ConfigFilePath);
    if (!configFile.exists()) {
        configFile.open(QIODevice::WriteOnly);
    }
}

#endif // EMOJIRUNNER_UTILITIES_H

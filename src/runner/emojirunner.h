#ifndef EMOJIRUNNER_H
#define EMOJIRUNNER_H

#include <KRunner/AbstractRunner>
#define NO_GUI
#include "core/EmojiCategory.h"
#include <QtCore>
// For autotyping
extern "C" {
#include <xdo.h>
}


class EmojiRunner : public Plasma::AbstractRunner {
Q_OBJECT
public:
    ~EmojiRunner() override;

public:
    EmojiRunner(QObject *parent, const QVariantList &args);

    const QString customEmojiFile = QDir::homePath() + "/.local/share/emojirunner/customemojis.json";
    QRegExp prefixRegex = QRegExp(R"(emoji(?: +(.*))?)");
    QList<EmojiCategory> emojiCategories;
    EmojiCategory favouriteCategory;
    QFileSystemWatcher watcher;
    bool tagSearchEnabled, descriptionSearchEnabled, globalSearchEnabled, singleRunnerModePaste = false;

    Plasma::QueryMatch createQueryMatch(const Emoji &emoji, double relevance);

    xdo_t *xdo = xdo_new(nullptr);

public: // Plasma::AbstractRunner API

    void match(Plasma::RunnerContext &context) override;

    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

    void emitCTRLV();

public Q_SLOTS:

    void reloadPluginConfiguration(const QString &configFile = "");
};

#endif

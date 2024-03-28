#ifndef EMOJIRUNNER_H
#define EMOJIRUNNER_H

#include "core/EmojiCategory.h"
#include <KRunner/AbstractRunner>
#include <QFileSystemWatcher>
#include <krunner_version.h>

#if KRUNNER_VERSION_MAJOR == 5
#include <QAction>
#else
#include <KRunner/Action>
#endif

struct xdo;
class EmojiRunner : public KRunner::AbstractRunner
{
    Q_OBJECT
public:
    EmojiRunner(QObject *parent, const KPluginMetaData &pluginMetaData, const QVariantList &args);
    ~EmojiRunner() override;

    const QRegularExpression prefixRegex{QStringLiteral(R"(^emoji(?: +(.*))?$)")};
    QList<EmojiCategory> emojiCategories;
    EmojiCategory favouriteCategory;
    QFileSystemWatcher watcher;
    int pasteTimeout;
    bool tagSearchEnabled, descriptionSearchEnabled, globalSearchEnabled;
    const QLatin1String queryPrefix{"emoji"};
#if KRUNNER_VERSION_MAJOR == 5
    QList<QAction *> matchActionList;
#else
    KRunner::Actions matchActionList;
#endif

    KRunner::QueryMatch createQueryMatch(const Emoji &emoji, qreal relevance, bool isExactMatch = false);

#ifdef XDO_LIB
    xdo *xdo;
#endif

    /**
     * Emits the Ctrl+V key combination. If the xdo.h file was available at compile time it uses the api,
     * otherwise it starts a new xdotool process
     */
    void emitCTRLV();

public: // Plasma::AbstractRunner API
    void match(KRunner::RunnerContext &context) override;
    void run(const KRunner::RunnerContext &context, const KRunner::QueryMatch &match) override;

public Q_SLOTS:
    void reloadPluginConfiguration(const QString &configFile = QString());
};

#endif

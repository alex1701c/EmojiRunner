#ifndef EMOJIRUNNER_H
#define EMOJIRUNNER_H

#include "core/EmojiCategory.h"
#include <KRunner/AbstractRunner>
#include <QAction>
#include <QtCore>
#include <krunner_version.h>

#ifdef XDO_LIB
// For autotyping
extern "C" {
#include <xdo.h>
}
#endif

class EmojiRunner : public Plasma::AbstractRunner
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
    QList<QAction *> matchActionList;

    Plasma::QueryMatch createQueryMatch(const Emoji &emoji, qreal relevance, Plasma::QueryMatch::Type type = Plasma::QueryMatch::CompletionMatch);

#ifdef XDO_LIB
    xdo_t *xdo = xdo_new(nullptr);

#endif

    /**
     * Emits the Ctrl+V key combination. If the xdo.h file was available at compile time it uses the api,
     * otherwise it starts a new xdotool process
     */
    void emitCTRLV();

public: // Plasma::AbstractRunner API
    void match(Plasma::RunnerContext &context) override;
    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;

public Q_SLOTS:
    void reloadPluginConfiguration(const QString &configFile = QString());
};

#endif

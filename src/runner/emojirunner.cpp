#include "emojirunner.h"
#include "core/FileReader.h"
#include "core/utilities.h"
#include "core/Config.h"

#include <KLocalizedString>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include <krunner_version.h>

#include <QApplication>
#include <QClipboard>

EmojiRunner::EmojiRunner(QObject *parent, const KPluginMetaData &pluginMetaData, const QVariantList &args)
        : Plasma::AbstractRunner(parent, pluginMetaData, args) {
    setObjectName(QStringLiteral("EmojiRunner"));
    // Add file watcher for config
    createConfigFile();
    watcher.addPath(Config::ConfigFilePath);
    watcher.addPath(Config::CustomEmojiFilePath);
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &EmojiRunner::reloadPluginConfiguration);
    reloadPluginConfiguration();
}

EmojiRunner::~EmojiRunner() {
#ifdef XDO_LIB
    xdo_free(xdo);
#endif
}

void EmojiRunner::reloadPluginConfiguration(const QString &configFile) {
    KConfigGroup config = KSharedConfig::openConfig(Config::ConfigFilePath)->group(Config::RootGroup);
    // Force sync from file
    if (!configFile.isEmpty()) config.config()->reparseConfiguration();

    // If the file gets edited with a text editor, it often gets replaced by the edited version
    // https://stackoverflow.com/a/30076119/9342842
    if (!configFile.isEmpty()) {
        watcher.addPath(configFile);
    }

    FileReader reader(config);
    emojiCategories = reader.getEmojiCategories(false);

    globalSearchEnabled = config.readEntry(Config::GlobalSearch, true);
    tagSearchEnabled = config.readEntry(Config::SearchByTags, false);
    descriptionSearchEnabled = config.readEntry(Config::SearchByDescription, false);

    // Items only change in reload config method => read once and reuse
    for (const auto &category: qAsConst(emojiCategories)) {
        if (category.name == Config::FavouritesCategory) {
            favouriteCategory = category;
            break;
        }
    }

    pasteTimeout = config.readEntry(Config::PasteTimeout, 100);
#ifndef XDO_LIB
    if (!QStandardPaths::findExecutable(QStringLiteral("xdotool")).isEmpty())
#endif
        matchActionList = {new QAction(QIcon::fromTheme("edit-paste"), "Paste emoji", this)};
}

void EmojiRunner::match(Plasma::RunnerContext &context) {
    QString search = context.query();
    const bool prefixed = search.startsWith(queryPrefix);

    if (prefixed) {
        const auto match = prefixRegex.match(search);
        if (!match.hasMatch()) return;
        search = match.captured(1).simplified();
    }

    QList<Plasma::QueryMatch> matches;
    if (prefixed && search.isEmpty()) {
        for (const auto &emoji : qAsConst(favouriteCategory.emojis)) {
            matches.append(createQueryMatch(emoji, (float) emoji.favourite / 21,
                    Plasma::QueryMatch::ExactMatch));
        }
    } else if (prefixed || (globalSearchEnabled && search.count() > 2) || context.singleRunnerQueryMode()) {
        for (const auto &category: qAsConst(emojiCategories)) {
            if (category.name == Config::FavouritesCategory) continue;
            for (const auto &emoji: category.emojis) {
                const double relevance = emoji.getEmojiRelevance(search, tagSearchEnabled, descriptionSearchEnabled);
                if (relevance == -1) continue;
                matches.append(createQueryMatch(emoji, relevance, prefixed ? Plasma::QueryMatch::ExactMatch : Plasma::QueryMatch::CompletionMatch));
            }
        }
    }
    context.addMatches(matches);
}

void EmojiRunner::run(const Plasma::RunnerContext &/*context*/, const Plasma::QueryMatch &match) {
    QApplication::clipboard()->setText(match.text());
    if (match.selectedAction()) {
        // Wait for krunner to be closed before typing
        QTimer::singleShot(pasteTimeout, this, [this]() {
            emitCTRLV();
        });

    }
}


Plasma::QueryMatch EmojiRunner::createQueryMatch(const Emoji &emoji, const qreal relevance, Plasma::QueryMatch::Type type){
    Plasma::QueryMatch match(this);
    match.setText(emoji.emoji);
    match.setType(type);
    match.setSubtext(emoji.name);
    match.setData(emoji.emoji);
    match.setRelevance(relevance);
    return match;
}

void EmojiRunner::emitCTRLV() {
#ifdef XDO_LIB
    // Emit Ctrl+V to paste clipboard content
    xdo_send_keysequence_window(xdo, CURRENTWINDOW, "ctrl+v", 0);
#else
    // Works but is slower and starts new process
    QProcess::startDetached("xdotool", QStringList() << "key" << "ctrl+v");

    // Does not always work
    // QProcess::startDetached("bash", QStringList() << "-c" << "sleep 0.2; xdotool type \"" + match.text() + "\"");
#endif
}

K_EXPORT_PLASMA_RUNNER_WITH_JSON(EmojiRunner, "emojirunner.json")

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "emojirunner.moc"

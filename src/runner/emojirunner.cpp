#include "emojirunner.h"
#include "core/Config.h"
#include "core/FileReader.h"
#include "core/utilities.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <krunner_version.h>

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QTimer>

#ifdef XDO_LIB
// For autotyping
extern "C" {
#include <xdo.h>
}
#endif

EmojiRunner::EmojiRunner(QObject *parent, const KPluginMetaData &pluginMetaData, const QVariantList &args)
#if KRUNNER_VERSION_MAJOR == 5
    : KRunner::AbstractRunner(parent, pluginMetaData, args)
#else
    : KRunner::AbstractRunner(parent, pluginMetaData)
#endif
{
    // Add file watcher for config
    createConfigFile();
    watcher.addPath(Config::ConfigFilePath);
    watcher.addPath(Config::CustomEmojiFilePath);
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &EmojiRunner::reloadPluginConfiguration);
    reloadPluginConfiguration();
#ifdef XDO_LIB
    xdo = xdo_new(nullptr);
#endif
}

EmojiRunner::~EmojiRunner()
{
#ifdef XDO_LIB
    xdo_free(xdo);
#endif
}

void EmojiRunner::reloadPluginConfiguration(const QString &configFile)
{
    KConfigGroup config = KSharedConfig::openConfig(Config::ConfigFilePath)->group(Config::RootGroup);
    // Force sync from file
    if (!configFile.isEmpty())
        config.config()->reparseConfiguration();

    // If the file gets edited with a text editor, it often gets replaced by the edited version
    // https://stackoverflow.com/a/30076119/9342842
    if (!configFile.isEmpty()) {
        watcher.addPath(configFile);
    }

    FileReader reader(config);
    emojiCategories = reader.getEmojiCategories(false);

    globalSearchEnabled = config.readEntry(Config::GlobalSearch, true);
    tagSearchEnabled = config.readEntry(Config::SearchByTags, true);
    descriptionSearchEnabled = config.readEntry(Config::SearchByDescription, false);

    // Items only change in reload config method => read once and reuse
    for (const auto &category : qAsConst(emojiCategories)) {
        if (category.name == Config::FavouritesCategory) {
            favouriteCategory = category;
            break;
        }
    }

    pasteTimeout = config.readEntry(Config::PasteTimeout, 100);
#ifndef XDO_LIB
    if (!QStandardPaths::findExecutable(QStringLiteral("xdotool")).isEmpty())
#endif
#if KRUNNER_VERSION_MAJOR == 5
        matchActionList = {new QAction(QIcon::fromTheme("edit-paste"), "Paste emoji", this)};
#else
    matchActionList = {KRunner::Action("paste", "edit-paste", "Paste emoji")};
#endif

    QList<KRunner::RunnerSyntax> syntaxes;
    syntaxes.append(KRunner::RunnerSyntax(QStringLiteral("emoji :q:"),
                                          QStringLiteral("Searches for emojis matching the query. If no query is specified, the favourites are shown")));
    if (globalSearchEnabled) {
        syntaxes.append(
            KRunner::RunnerSyntax(QStringLiteral(":q:"),
                                  QStringLiteral("Searches for emojis matching the query without requiring a keyword. This can be disabled in the settings")));
    }
    setSyntaxes(syntaxes);
}

void EmojiRunner::match(KRunner::RunnerContext &context)
{
    QString search = context.query();
    const bool prefixed = search.startsWith(queryPrefix);

    if (prefixed) {
        const auto match = prefixRegex.match(search);
        if (!match.hasMatch())
            return;
        search = match.captured(1).simplified();
    }

    QList<KRunner::QueryMatch> matches;
    if (prefixed && search.isEmpty()) {
        for (const auto &emoji : qAsConst(favouriteCategory.emojis)) {
            matches.append(createQueryMatch(emoji, (float)emoji.favourite / 21, true));
        }
    } else if (prefixed || (globalSearchEnabled && search.count() > 2) || context.singleRunnerQueryMode()) {
        for (const auto &category : qAsConst(emojiCategories)) {
            if (category.name == Config::FavouritesCategory)
                continue;
            for (const auto &emoji : category.emojis) {
                const double relevance = emoji.getEmojiRelevance(search, tagSearchEnabled, descriptionSearchEnabled);
                if (relevance == -1)
                    continue;
                matches.append(createQueryMatch(emoji, relevance, prefixed));
            }
        }
    }
    context.addMatches(matches);
}

void EmojiRunner::run(const KRunner::RunnerContext & /*context*/, const KRunner::QueryMatch &match)
{
    QApplication::clipboard()->setText(match.text());
    if (match.selectedAction()) {
        // Wait for krunner to be closed before typing
        QTimer::singleShot(pasteTimeout, this, [this]() {
            emitCTRLV();
        });
    }
}

KRunner::QueryMatch EmojiRunner::createQueryMatch(const Emoji &emoji, const qreal relevance, bool isExactMatch)
{
    KRunner::QueryMatch match(this);
    match.setText(emoji.emoji);
#if KRUNNER_VERSION < QT_VERSION_CHECK(5, 113, 0)
    match.setType(isExactMatch ? KRunner::QueryMatch::ExactMatch : KRunner::QueryMatch::CompletionMatch);
#else
    match.setCategoryRelevance(isExactMatch ? KRunner::QueryMatch::CategoryRelevance::Highest : KRunner::QueryMatch::CategoryRelevance::Moderate);
#endif
    match.setSubtext(emoji.name);
    match.setData(emoji.emoji);
    match.setRelevance(relevance);
    match.setActions(matchActionList);
    return match;
}

void EmojiRunner::emitCTRLV()
{
#ifdef XDO_LIB
    // Emit Ctrl+V to paste clipboard content
    xdo_send_keysequence_window(xdo, CURRENTWINDOW, "ctrl+v", 0);
#else
    // Works but is slower and starts new process
    QProcess::startDetached("xdotool", QStringList{"key", "ctrl+v"});

    // Does not always work
    // QProcess::startDetached("bash", QStringList{"-c", "sleep 0.2; xdotool type \"" + match.text() + "\""});
#endif
}

K_PLUGIN_CLASS_WITH_JSON(EmojiRunner, "emojirunner.json")

#include "emojirunner.moc"
#include "moc_emojirunner.cpp"

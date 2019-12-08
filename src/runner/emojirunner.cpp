#include "emojirunner.h"
#include "core/FileReader.h"
#include "core/Config.h"

#include <KLocalizedString>
#include <KConfigCore/KConfig>
#include <KConfigCore/KSharedConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QClipboard>
#include <QDebug>


EmojiRunner::EmojiRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent, args) {
    setObjectName(QStringLiteral("EmojiRunner"));
    setIgnoredTypes(Plasma::RunnerContext::NetworkLocation);

    const QString configFolder = QDir::homePath() + "/.config/krunnerplugins/";
    const QDir configDir(configFolder);
    if (!configDir.exists()) configDir.mkpath(configFolder);
    // Create file
    QFile configFile(configFolder + Config::ConfigFileName);
    if (!configFile.exists()) {
        configFile.open(QIODevice::WriteOnly);
        configFile.close();
    }
    // Add file watcher for config
    watcher.addPath(configFolder + Config::ConfigFileName);
    connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadPluginConfiguration(QString)));
    if (QFile::exists(customEmojiFile)) watcher.addPath(customEmojiFile);
    reloadPluginConfiguration();
}

EmojiRunner::~EmojiRunner() {
#ifdef XDO_LIB
    xdo_free(xdo);
#endif

}

void EmojiRunner::reloadPluginConfiguration(const QString &configFile) {
    KConfigGroup config = KSharedConfig::openConfig(QDir::homePath() + "/.config/krunnerplugins/" + Config::ConfigFileName)
            ->group(Config::RootGroup);
    // Force sync from file
    if (!configFile.isEmpty()) config.config()->reparseConfiguration();

    // If the file gets edited with a text editor, it often gets replaced by the edited version
    // https://stackoverflow.com/a/30076119/9342842
    if (!configFile.isEmpty()) {
        if (QFile::exists(configFile)) {
            watcher.addPath(configFile);
        }
        if (configFile != customEmojiFile && QFile::exists(customEmojiFile)) watcher.addPath(customEmojiFile);
    }

    FileReader reader(config);
    emojiCategories = reader.getEmojiCategories(false);

    globalSearchEnabled = config.readEntry(Config::GlobalSearch, true);
    tagSearchEnabled = config.readEntry(Config::SearchByTags, false);
    descriptionSearchEnabled = config.readEntry(Config::SearchByDescription, false);
    singleRunnerModePaste = config.readEntry(Config::SingleRunnerModePaste, true);

    // Items only change in reload config method => read once and reuse
    for (const auto &category:emojiCategories) {
        if (category.name == Config::FavouritesCategory) {
            favouriteCategory = category;
            break;
        }
    }
}

void EmojiRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;

#ifdef stage_dev
    int total = 0;
    for (const auto &c:emojiCategories) {
        total += c.emojis.count();
    }
    qInfo() << total;
#endif
    const auto term = QString(context.query()).replace(QString::fromWCharArray(L"\u001B"), " ").toLower();// Remove escape character
    const bool prefixed = term.startsWith("emoji");
    if (!globalSearchEnabled && !prefixed) return;

    QString search = term;
    if (prefixed) {
        prefixRegex.indexIn(term);
        search = prefixRegex.cap(1).simplified();
    }
    QList<Plasma::QueryMatch> matches;
    if (prefixed && search.isEmpty()) {
        for (const auto &emoji :favouriteCategory.emojis.values()) {
            matches.append(createQueryMatch(emoji, emoji.favourite / 21));
        }
    } else if (prefixed || globalSearchEnabled || context.singleRunnerQueryMode()) {
        for (const auto &category:emojiCategories) {
            if (category.name == Config::FavouritesCategory) continue;
            for (const auto &emoji :category.emojis.values()) {
                const double relevance = emoji.getEmojiRelevance(search, tagSearchEnabled, descriptionSearchEnabled);
                if (relevance == -1) continue;
                matches.append(createQueryMatch(emoji, relevance));
            }
        }
    }
    context.addMatches(matches);
}

void EmojiRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)

    QApplication::clipboard()->setText(match.text());
    if (context.singleRunnerQueryMode() && singleRunnerModePaste) {

        // Wait for krunner to be closed before typing
        QTimer::singleShot(10, this, [this]() {
            emitCTRLV();
        });

    }
}

Plasma::QueryMatch EmojiRunner::createQueryMatch(const Emoji &emoji, const qreal relevance) {
    Plasma::QueryMatch match(this);
    match.setText(emoji.emoji);
#ifndef stage_dev
    match.setSubtext(emoji.name);
#else
    match.setSubtext(QString(emoji.name).replace("_", " ") + " ---- " + QString::number(relevance));
#endif
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


K_EXPORT_PLASMA_RUNNER(emojirunner, EmojiRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "emojirunner.moc"

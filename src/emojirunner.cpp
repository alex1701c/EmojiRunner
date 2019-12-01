#include "emojirunner.h"
#include "FileReader.h"

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
    QFile configFile(configFolder + "emojirunnerrc");
    if (!configFile.exists()) {
        configFile.open(QIODevice::WriteOnly);
        configFile.close();
    }
    // Add file watcher for config
    watcher.addPath(configFolder + "emojirunnerrc");
    connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(reloadPluginConfiguration(QString)));
    // customemojis.json file
    if (QFile::exists(customEmojiFile)) watcher.addPath(customEmojiFile);
    reloadPluginConfiguration();
}

void EmojiRunner::reloadPluginConfiguration(const QString &configFile) {
    KConfigGroup config = KSharedConfig::openConfig(QDir::homePath() + "/.config/krunnerplugins/emojirunnerrc")
            ->group("Config");
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

    globalSearchEnabled = config.readEntry("globalSearch", "true") == "true";
    tagSearchEnabled = config.readEntry("searchByTags", "false") == "true";
    descriptionSearchEnabled = config.readEntry("searchByDescription", "false") == "true";
    singleRunnerModePaste = config.readEntry("singleRunnerModePaste", "true") == "true";

    // Items only change in reload config method => read once and reuse
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") {
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
        search = prefixRegex.capturedTexts().at(1);
    }
    QList<Plasma::QueryMatch> matches;
    if (prefixed && search.isEmpty()) {
        for (const auto &emoji :favouriteCategory.emojis.values()) {
            matches.append(createQueryMatch(emoji, (float) emoji.favourite / 21));
        }
    } else if (prefixed || globalSearchEnabled || context.singleRunnerQueryMode()) {
        for (const auto &category:emojiCategories) {
            if (category.name == "Favourites") continue;
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
        // Does not always work
        // QProcess::startDetached("bash", QStringList() << "-c" << "sleep 0.2; xdotool type \"" + match.text() + "\"");
        QProcess::startDetached("sh", QStringList() << "-c" << "xdotool key ctrl+v");
    }
}

Plasma::QueryMatch EmojiRunner::createQueryMatch(const Emoji &emoji, const double relevance) {
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

K_EXPORT_PLASMA_RUNNER(emojirunner, EmojiRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "emojirunner.moc"

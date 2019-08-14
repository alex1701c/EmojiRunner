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
}

void EmojiRunner::reloadConfiguration() {
    emojiCategories = FileReader::readJSONFile();

    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");

    globalSearchEnabled = config.readEntry("globalSearch", "true") == "true";
    tagSearchEnabled = config.readEntry("searchByTags", "false") == "true";
    descriptionSearchEnabled = config.readEntry("searchByDescription", "false") == "true";
}

void EmojiRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;

    QList<Plasma::QueryMatch> matches;
#ifdef stage_dev
    int total = 0;
    for (const auto &c:emojiCategories) {
        total += c.emojis.count();
    }
    qInfo() << total;
#endif
    const auto term = QString(context.query()).replace(QString::fromWCharArray(L"\u001B"), " ");// Remove escape character
    const bool prefixed = term.startsWith("emoji");
    QRegExp regex(R"(emoji(?: +(.*))?)");
    regex.indexIn(term);
    QString search = regex.capturedTexts().last();
    if (!prefixed) search = term;

    if (prefixed && search.isEmpty()) {
        // region favourites
        EmojiCategory favouriteCategory;
        for (const auto &category:emojiCategories) {
            if (category.name == "Favourites") {
                favouriteCategory = category;
                break;
            }
        }
        for (const auto &key :favouriteCategory.emojis.keys()) {
            const auto emoji = favouriteCategory.emojis.value(key);
            matches.append(createQueryMatch(emoji, (float) emoji.favourite / 21));
        }
        // endregion
    } else if (prefixed || globalSearchEnabled || context.singleRunnerQueryMode()) {
        // region search: emoji <query>
        for (const auto &category:emojiCategories) {
            if (!category.enabled || category.name == "Favourites") continue;
            for (const auto &key :category.emojis.keys()) {
                const auto emoji = category.emojis.value(key);
                double relevance = -1;

                if (emoji.nameQueryMatches(search)) relevance = (double) search.length() / (key.length() * 8);
                else if (tagSearchEnabled) relevance = emoji.tagsQueryMatches(search);
                if (descriptionSearchEnabled && relevance == -1) relevance = emoji.descriptionQueryMatches(search);

                if (relevance == -1) continue;
                if (emoji.favourite != 0) relevance += 0.5;
                if (category.name == "Smileys & Emotion") relevance *= 2;
                matches.append(createQueryMatch(emoji, relevance));
            }
        }
        //endregion
    }
    context.addMatches(matches);
}

void EmojiRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) {
    Q_UNUSED(context)

    QApplication::clipboard()->setText(match.text());
}

K_EXPORT_PLASMA_RUNNER(emojirunner, EmojiRunner)

// needed for the QObject subclass declared as part of K_EXPORT_PLASMA_RUNNER
#include "emojirunner.moc"

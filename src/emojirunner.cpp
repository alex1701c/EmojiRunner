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
    setIgnoredTypes(Plasma::RunnerContext::Directory |
                    Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation
    );
}

EmojiRunner::~EmojiRunner() = default;

void EmojiRunner::reloadConfiguration() {
    emojiCategories = FileReader::readJSONFile();

    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");

    // TODO How to store favourites
    // Default favourites (ids)
}

void EmojiRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;

    QList<Plasma::QueryMatch> matches;
    const auto term = QString(context.query()).replace(QString::fromWCharArray(L"\u001B"), " ");
    const bool globalSearch = !term.startsWith("emoji");
    QRegExp regex(R"(emoji(?: +(.*))?)");
    regex.indexIn(term);
    const QString search = regex.capturedTexts().last();
    if (!globalSearch && search.isEmpty()) {
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
            matches.append(createQueryMatch(emoji, (float) emoji.favourite / 22));
        }
        // endregion
    } else if (!globalSearch) {
        // region search: emoji <query>
        for (const auto &category:emojiCategories) {
            if (!category.enabled || category.name == "Favourites") continue;
            for (const auto &key :category.emojis.keys()) {
                if (nameQueryMatches(key, search)) {
                    const Emoji emoji = category.emojis.value(key);
                    float relevance = (float) search.length() / (key.length() * 8);
                    if (category.name == "Smileys & Emotion") relevance *= 4;
                    if (emoji.favourite != 0) relevance += 0.5;
                    matches.append(createQueryMatch(emoji, relevance));
                }
            }
        }
        //endregion
    } else if (config.readEntry("globalSearch", "true") == "true") {
        //region global search
        // Search all categories
        for (const auto &category:emojiCategories) {
            if (!category.enabled || category.name == "Favourites") continue;
            for (const auto &key :category.emojis.keys()) {
                if (nameQueryMatches(key, term)) {
                    float relevance = (float) term.length() / (key.length() * 2);
                    if (category.name == "Smileys & Emotion") relevance *= 2;
                    matches.append(createQueryMatch(category.emojis.value(key), relevance));
                }
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

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
    //7;1;37;14;18;154;77;36;10;111;59;23;33;87;167
}

void EmojiRunner::match(Plasma::RunnerContext &context) {
    if (!context.isValid()) return;


    QList<Plasma::QueryMatch> matches;
    const auto term = context.query();

    if (term.startsWith("emoji")) {
        // TODO Display favourites or other search results where favourites have a higher priority
    }

    // Search all categories
    if (config.readEntry("globalSearch", "true") == "true") {
        for (const auto &category:emojiCategories) {
            for (const auto &key :category.emojis.keys()) {
                const auto key2 = QString(key).replace("_", "");
                if (key2.startsWith(term, Qt::CaseInsensitive) ||
                    QString(key).replace("_", "").startsWith(term, Qt::CaseInsensitive)) {
                    Plasma::QueryMatch match(this);
                    match.setText(category.emojis.value(key).emoji);
                    match.setSubtext(key2);
                    match.setData(category.emojis.value(key).emoji);
                    // Emoji should have higher priority than the symbols
                    float relevance = (float) term.length() / (key2.length() * 2);
                    if (category.name == "Smileys & Emotion") relevance *= 2;
                    match.setRelevance(relevance);
                    matches.append(match);
                }
            }
        }
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

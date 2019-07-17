#include "emojirunner.h"
#include "FileReader.h"

#include <KLocalizedString>
#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>

#include <QApplication>
#include <QClipboard>


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
}

void EmojiRunner::match(Plasma::RunnerContext &context) {
    QList<Plasma::QueryMatch> matches;
    const auto term = context.query();

    for (const auto &key:emojiCategories.at(6).emojis.keys()) {
        if (key.startsWith(term)) {
            Plasma::QueryMatch match(this);
            match.setIconName("preferences-desktop-font");
            match.setText(emojiCategories.at(6).emojis.value(key).emoji);
            match.setSubtext(emojiCategories.at(6).emojis.value(key).description);
            match.setData(emojiCategories.at(6).emojis.value(key).emoji);
            matches.append(match);
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

#ifndef EMOJIRUNNER_H
#define EMOJIRUNNER_H

#include <KRunner/AbstractRunner>
#include "EmojiCategory.h"

class EmojiRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    EmojiRunner(QObject *parent, const QVariantList &args);

    ~EmojiRunner() override;

    QList<EmojiCategory> emojiCategories;

    KConfigGroup config;

    static bool nameQueryMatches(const QString &key, const QString &search) {
        return QString(key).replace("_", "").startsWith(search, Qt::CaseInsensitive) ||
               QString(key).replace("_", " ").startsWith(search, Qt::CaseInsensitive);
    }

    Plasma::QueryMatch createQueryMatch(const Emoji &emoji, float relevance) {
        Plasma::QueryMatch match(this);
        match.setText(emoji.emoji);
        match.setSubtext(QString(emoji.name).replace("_", " "));
        match.setData(emoji.emoji);
        match.setRelevance(relevance);
        return match;
    }

public: // Plasma::AbstractRunner API
    void reloadConfiguration() override;

    void match(Plasma::RunnerContext &context) override;

    void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match) override;
};

#endif

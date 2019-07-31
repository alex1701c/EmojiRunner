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

    bool tagSearchEnabled, descriptionSearchEnabled = false;

    static bool nameQueryMatches(const QString &key, const QString &search) {
        return QString(key).replace("_", "").startsWith(search, Qt::CaseInsensitive) ||
               QString(key).replace("_", " ").startsWith(search, Qt::CaseInsensitive);
    }

    static double tagsQueryMatches(const QString &search, const Emoji &emoji) {
        for (const auto &tag:emoji.tags) {
            if (tag.contains(search, Qt::CaseInsensitive)) {
                double relevance = (double) search.length() / (tag.length() * 8);
                return relevance;
            }
        }

        return -1;
    }

    static double descriptionQueryMatches(const QString &search, const Emoji &emoji) {
        if (emoji.description.contains(search, Qt::CaseInsensitive)) {
            return (double) search.length() / (emoji.description.length() * 8);
        }
        return -1;
    }

    Plasma::QueryMatch createQueryMatch(const Emoji &emoji, double relevance) {
        Plasma::QueryMatch match(this);
        match.setText(emoji.emoji);
#ifndef stage_dev
        match.setSubtext(QString(emoji.name).replace("_", " "));
#else
        match.setSubtext(QString(emoji.name).replace("_", " ") + " ---- " + QString::number(relevance));
#endif
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

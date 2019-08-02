#ifndef EMOJIRUNNER_H
#define EMOJIRUNNER_H

#include <KRunner/AbstractRunner>
#include "EmojiCategory.h"

class EmojiRunner : public Plasma::AbstractRunner {
Q_OBJECT

public:
    EmojiRunner(QObject *parent, const QVariantList &args);

    QList<EmojiCategory> emojiCategories;

    KConfigGroup config;

    bool tagSearchEnabled, descriptionSearchEnabled, globalSearchEnabled = false;

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

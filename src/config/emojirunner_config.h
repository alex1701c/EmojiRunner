#ifndef EmojiRunnerCONFIG_H
#define EmojiRunnerCONFIG_H

#include "ui_emojirunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>

class EmojiRunnerConfigForm : public QWidget, public Ui::EmojiRunnerConfigUi {
Q_OBJECT

public:
    explicit EmojiRunnerConfigForm(QWidget *parent);
};

class EmojiRunnerConfig : public KCModule {
Q_OBJECT

public:
    explicit EmojiRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

public Q_SLOTS:

    void save() override;

    void load() override;

    void defaults() override;


private:
    EmojiRunnerConfigForm *m_ui;

};

#endif

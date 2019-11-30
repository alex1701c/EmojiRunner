#ifndef EmojiRunnerCONFIG_H
#define EmojiRunnerCONFIG_H

#include "ui_emojirunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>
#include <EmojiCategory.h>
#include "ui_emojirunner_popup.h"
#include "emojirunner_popup.h"

class EmojiRunnerConfigForm : public QWidget, public Ui::EmojiRunnerConfigUi {
Q_OBJECT

public:
    explicit EmojiRunnerConfigForm(QWidget *parent);
};

class EmojiRunnerConfig : public KCModule {
Q_OBJECT

public:
    explicit EmojiRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

    KConfigGroup config;
    bool filterActive = false;
    float configUnicodeVersion;
    float configIosVersion;

    QList<EmojiCategory> emojiCategories;
    QMap<QString, Emoji> allEmojis;
    QStringList disabledEmojiCategoryNames;
    QStringList favouriteFilters = {"name"};

    void displayVisibleItems() const;

    void unhideAll();

public Q_SLOTS:

    void save() override;

    void load() override;

    void defaults() override;

    void filterEmojiListView();

    void filtersChanged(bool reloadFilter = true);

    void unicodeVersionChanged();

    void iosVersionChanged();

    void categoriesChanged();

    void showOnlyFavourites();

    void validateMoveFavouriteButtons();

    void moveFavouriteUp();

    void moveFavouriteDown();

    void checkMaxFavourites();

    void changeFontSize(int value);

    void addEmoji();

    void editEmoji();

    void applyEmojiPopupResults(const Emoji &emoji, const QString &originalName = "");

private:
    EmojiRunnerConfigForm *m_ui;

};

#endif

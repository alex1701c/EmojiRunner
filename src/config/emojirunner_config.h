#ifndef EmojiRunnerCONFIG_H
#define EmojiRunnerCONFIG_H

#include "ui_emojirunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>
#include <EmojiCategory.h>

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
    QStringList disabledEmojis;
    QList<float> unicodeVersions = {3.0, 3.2, 4.0, 4.1, 5.1, 5.2, 6.0, 6.1, 7.0, 8.0, 9.0, 11.0, 12.0};
    QList<float> iosVersions = {6.0, 8.3, 9.0, 9.1, 10.0, 10.2, 12.1, 13.0};
    QStringList favouriteFilters = {"name"};

    void displayVisibleItems() const {
        int visibleItems = 0;
        int count = this->m_ui->favouriteListView->count();
        for (int i = 0; i < count; ++i) {
            if (!this->m_ui->favouriteListView->item(i)->isHidden()) ++visibleItems;
        }

        m_ui->favouriteVisibleElements->setText(QString::number(visibleItems) + " Elements");
    }

    void unhideAll();

public Q_SLOTS:

    void save() override;

    void load() override;

    void defaults() override;

    void filterFavourites();

    void filtersChanged(bool reloadFilter = true);

    void unicodeVersionsChanged();

    void iosVersionsChanged();

    void categoriesApplyChanges();

    void showOnlyFavourites();

    void validateMoveFavouriteButtons();

    void moveFavouriteUp();

    void moveFavouriteDown();

    void saveFavourites();

private:
    EmojiRunnerConfigForm *m_ui;

};

#endif

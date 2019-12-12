#ifndef EmojiRunnerCONFIG_H
#define EmojiRunnerCONFIG_H

#include "ui_emojirunner_config.h"
#include <KCModule>
#include <KConfigCore/KConfigGroup>
#include <core/EmojiCategory.h>
#include "ui_emojirunner_popup.h"
#include "emojirunner_popup.h"

class EmojiRunnerConfigForm : public QWidget, public Ui::EmojiRunnerConfigUi {
Q_OBJECT

public:
    explicit EmojiRunnerConfigForm(QWidget *parent);
};

class EmojiRunnerConfig : public KCModule {
Q_OBJECT

private:
    EmojiRunnerConfigForm *m_ui;
    KConfigGroup config;
    bool filterActive = false;
    bool customEntriesExist = false;
    bool customFilterChecked = false;
    float configUnicodeVersion;
    float configIosVersion;
    int newFontSize;

    QList<EmojiCategory> emojiCategories;
    QMap<QString, Emoji> allEmojis;
    QStringList disabledEmojiCategoryNames;
    QStringList favouriteFilters = {"name"};

public:
    explicit EmojiRunnerConfig(QWidget *parent = nullptr, const QVariantList &args = QVariantList());

public Q_SLOTS:

    void save() override;

    void load() override;

    void defaults() override;

    void connectSignals();

    /**
     * Filters emojis based on filters and the search term
     */
    void filterEmojiListView();

    /**
     * Enable/Disable filter checkboxes and search with new filters
     * @param reloadFilter
     */
    void filtersChanged();

    /**
     * Check for newly enabled/disabled categories and add/remove the emojis
     */
    void categoriesChanged();

    /**
     * Toggle if only checked items are shown
     */
    void showOnlyFavourites();

    /**
     * Update configUnicodeVersion variable, clear filter text and call filterFavourites()
     */
    void unicodeVersionChanged();

    /**
     * Update configIosVersion variable, clear filter text and call filterFavourites()
     */
    void iosVersionChanged();

    /**
     * Show all items if they mathe the unicode version or if they are checked
     */
    void unhideAll();

    /**
     * Count visible items and update the value in the UI
     */
    void displayVisibleItems() const;

    /**
     * Checks if the number of favourites is greater than 20, if true it shows the maxFavouritesLabel with a warning
     */
    void checkMaxFavourites();

    /*
     * Change the font size based on the value
     */
    void changeFontSize(int value);

    /**
     * Show popup to add a new emoji
     */
    void addEmoji();

    /**
     * Show popup to edit an emoji
     */
    void editEmoji();

    /**
     * Deletes emoji from ListView
     */
    void deleteEmoji();

    /**
     * Applies the results of the popup to the emojiListView
     *
     * @param emoji
     * @param originalName
     */
    void applyEmojiPopupResults(const Emoji &emoji, const QString &originalName = "");

    /**
     * Enable/Disable the edit and delete buttons based on the current emoji of the emojiListView
     */
    void validateEditingOptions();

    /**
     * Hides/Unhides the search options for the emoji list view to give the results more space
     */
    void toggleFavouriteOptions();
};

#endif

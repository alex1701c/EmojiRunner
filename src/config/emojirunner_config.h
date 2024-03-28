#ifndef EmojiRunnerCONFIG_H
#define EmojiRunnerCONFIG_H

#include "ui_emojirunner_config.h"
#include "ui_emojirunner_popup.h"
#include <KCModule>
#include <KConfigGroup>
#include <core/EmojiCategory.h>

class EmojiRunnerConfigForm : public QWidget, public Ui::EmojiRunnerConfigUi
{
    Q_OBJECT

public:
    explicit EmojiRunnerConfigForm(QWidget *parent);
};

class EmojiRunnerConfig : public KCModule
{
    Q_OBJECT

private:
    EmojiRunnerConfigForm *m_ui;
    KConfigGroup config;
    bool customEntriesExist = false;
    bool customFilterWasChecked = false;
    float configUnicodeVersion;
    float configIosVersion;

    bool filterName = true;
    bool filterDescription = false;
    bool filterTags = false;
    bool filterCustom = false;

    QList<EmojiCategory> emojiCategories;
    QStringList disabledEmojiCategoryNames;

public:
    explicit EmojiRunnerConfig(QObject *parent, const QVariantList &args);

    ~EmojiRunnerConfig() override;

public Q_SLOTS:

    void save() override;

    void load() override;

    void defaults() override;

    /**
     * Connect all the signals and slots
     */
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
     * Count visible items and update the value in the UI
     */
    void displayVisibleItems();

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
    void applyEmojiPopupResults(const Emoji &emoji, int idx);

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

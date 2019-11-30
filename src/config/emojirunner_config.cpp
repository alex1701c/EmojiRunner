#include "emojirunner_config.h"
#include "emojirunner_popup.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <FileReader.h>
#include <QDebug>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QDialog>
#include <QtWidgets/QInputDialog>
#include <QJsonObject>
#include <QJsonArray>

K_PLUGIN_FACTORY(EmojiRunnerConfigFactory, registerPlugin<EmojiRunnerConfig>("kcm_krunner_emojirunner");)

EmojiRunnerConfigForm::EmojiRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

EmojiRunnerConfig::EmojiRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new EmojiRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    config = KSharedConfig::openConfig(QDir::homePath() + "/.config/krunnerplugins/emojirunnerrc")
            ->group("Config");
    config.config()->reparseConfiguration();

    FileReader reader(config);
    emojiCategories = reader.getEmojiCategories(true);
    disabledEmojiCategoryNames = reader.disabledCategories;

    std::sort(emojiCategories.begin(), emojiCategories.end(), [](const EmojiCategory &c1, const EmojiCategory &c2) -> bool {
        if (c1.name == "Smileys & Emotion") return true;
        return c1.name < c2.name;
    });

    // Connect slots for filters
    connect(m_ui->emojiListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(changed()));
    connect(m_ui->enableGlobalSearch, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->singleRunnerModePaste, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->favouriteFilter, SIGNAL(textChanged(QString)), this, SLOT(filterEmojiListView()));
    connect(m_ui->favouriteFilterName, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterDescription, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterTags, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterDescription_2, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->favouriteFilterTags_2, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->favouriteFilterCustom, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->favouriteFilterCustom, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    // Unicode Versions change
    connect(m_ui->unicodeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unicodeVersionChanged()));
    connect(m_ui->unicodeComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(changed()));
    connect(m_ui->iosComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(iosVersionChanged()));
    connect(m_ui->iosComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(changed()));
    // Disable categories
    connect(m_ui->categoryListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(changed()));
    connect(m_ui->categoryListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(categoriesChanged()));
    // Sort favourites
    connect(m_ui->sortFavourites, SIGNAL(clicked(bool)), this, SLOT(showOnlyFavourites()));
    connect(m_ui->sortFavourites, SIGNAL(clicked(bool)), this, SLOT(validateMoveFavouriteButtons()));
    connect(m_ui->emojiListView, SIGNAL(itemSelectionChanged()), this, SLOT(validateMoveFavouriteButtons()));
    connect(m_ui->emojiListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(validateMoveFavouriteButtons()));
    connect(m_ui->emojiListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(checkMaxFavourites()));
    // Move favourites up/down
    connect(m_ui->moveFavouriteUp, SIGNAL(clicked(bool)), this, SLOT(moveFavouriteUp()));
    connect(m_ui->moveFavouriteUp, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->moveFavouriteDown, SIGNAL(clicked(bool)), this, SLOT(moveFavouriteDown()));
    connect(m_ui->moveFavouriteDown, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Slider for font size
    connect(m_ui->fontSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(m_ui->fontSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(changeFontSize(int)));
    // Buttons for adding/updating/deleting emojis
    connect(m_ui->emojiListView, SIGNAL(currentRowChanged(int)), SLOT(validateEditingOptions()));
    connect(m_ui->addEmojiPushButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->addEmojiPushButton, SIGNAL(clicked(bool)), this, SLOT(addEmoji()));
    connect(m_ui->editEmojiPushButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->editEmojiPushButton, SIGNAL(clicked(bool)), this, SLOT(editEmoji()));
    connect(m_ui->deleteEmojiPushButton, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->deleteEmojiPushButton, SIGNAL(clicked(bool)), this, SLOT(deleteEmoji()));
}

void EmojiRunnerConfig::load() {
    m_ui->enableGlobalSearch->setChecked(config.readEntry("globalSearch", "true") == "true");
    m_ui->singleRunnerModePaste->setChecked(config.readEntry("singleRunnerModePaste", "true") == "true");
    m_ui->favouriteFilterDescription_2->setChecked(config.readEntry("searchByDescription", "false") == "true");
    m_ui->favouriteFilterTags_2->setChecked(config.readEntry("searchByTags", "false") == "true");

    // Load categories
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") continue;
        auto *item = new QListWidgetItem();
        item->setText(category.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (category.name == "Custom") customEntriesExist = true;
        item->setCheckState(disabledEmojiCategoryNames.contains(category.name) ? Qt::Unchecked : Qt::Checked);
        m_ui->categoryListView->addItem(item);
    }

    // Load favourites at top, apply sort from config
    QStringList favouriteNames;
    QList<Emoji> favouriteEmojisToAdd;
    for (const auto &category:emojiCategories) {
        if (category.name != "Favourites") continue;
        favouriteNames = category.emojis.keys();
        for (const auto &emoji:category.emojis.values()) {
            favouriteEmojisToAdd.append(emoji);
        }
    }
    std::sort(favouriteEmojisToAdd.begin(), favouriteEmojisToAdd.end(), [](const Emoji &e1, const Emoji &e2) -> bool {
        return e1.favourite > e2.favourite;
    });
    for (const auto &emoji:favouriteEmojisToAdd) {
        m_ui->emojiListView->addItem(emoji.toListWidgetItem());
    }

    // Load other emojis
    allEmojis.clear();
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") continue;
        for (const auto &emoji:category.emojis.values()) {
            allEmojis.insert(emoji.name, emoji);
            if (favouriteNames.contains(emoji.name)) continue;
            m_ui->emojiListView->addItem(emoji.toListWidgetItem());
        }
    }

    // Load versions
    m_ui->unicodeComboBox->addItems({"3.0", "3.2", "4.0", "4.1", "5.1", "5.2", "6.0", "6.1", "7.0", "8.0", "9.0", "11.0", "12.0"});
    m_ui->iosComboBox->addItems({"6.0", "8.3", "9.0", "9.1", "10.0", "10.2", "12.1", "13.0"});
    configUnicodeVersion = config.readEntry("unicodeVersion", "11").toFloat();
    configIosVersion = config.readEntry("iosVersion", "13").toFloat();
    m_ui->unicodeComboBox->setCurrentText(config.readEntry("unicodeVersion", "11.0"));
    m_ui->iosComboBox->setCurrentText(config.readEntry("iosVersion", "13.0"));

    categoriesChanged();
    filterActive = true;
    filterEmojiListView();
    validateMoveFavouriteButtons();
    validateEditingOptions();

    emit changed(true);
}


void EmojiRunnerConfig::save() {
    // Save general settings
    config.writeEntry("globalSearch", m_ui->enableGlobalSearch->isChecked() ? "true" : "false");
    config.writeEntry("singleRunnerModePaste", m_ui->singleRunnerModePaste->isChecked() ? "true" : "false");
    config.writeEntry("searchByTags", m_ui->favouriteFilterTags_2->isChecked() ? "true" : "false");
    config.writeEntry("searchByDescription", m_ui->favouriteFilterDescription_2->isChecked() ? "true" : "false");
    config.writeEntry("unicodeVersion", m_ui->unicodeComboBox->currentText());
    config.writeEntry("iosVersion", m_ui->iosComboBox->currentText());

    // Save disabled categories
    QString disabledCategories;
    const int categoryCount = m_ui->categoryListView->count();
    for (int i = 0; i < categoryCount; i++) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) disabledCategories.append(item->text() + ";");
    }
    config.writeEntry("disabledCategories", disabledCategories);

    QList<Emoji> customEmojis;
    // Save favourites and search for custom emojis
    const int emojiCount = m_ui->emojiListView->count();
    QString favouriteIDs;
    for (int i = 0; i < emojiCount; ++i) {
        const auto *item = m_ui->emojiListView->item(i);
        const auto emoji = allEmojis.value(item->data(1).toString());
        if (item->checkState() == Qt::Checked) {
            favouriteIDs.append(QString::number(emoji.id) + ";");
        }
        if (emoji.category == "Custom") customEmojis.append(emoji);
    }
    config.writeEntry("favourites", favouriteIDs);

    if (!customEmojis.isEmpty()) Emoji::writeToJSONFile(customEmojis);

    config.sync();

    emit changed();
}

void EmojiRunnerConfig::defaults() {
    m_ui->enableGlobalSearch->setChecked(true);
    m_ui->singleRunnerModePaste->setChecked(true);
    m_ui->favouriteFilterDescription_2->setChecked(false);
    m_ui->favouriteFilterTags_2->setChecked(false);
    m_ui->unicodeComboBox->setCurrentText("11");
    m_ui->iosComboBox->setCurrentText("13");

    const int categoryCount = m_ui->categoryListView->count();
    for (int i = 0; i < categoryCount; ++i) {
        m_ui->categoryListView->item(i)->setCheckState(Qt::Checked);
    }

    m_ui->favouriteFilter->clear();
    m_ui->favouriteFilterName->setChecked(true);
    m_ui->favouriteFilterTags->setChecked(false);
    m_ui->favouriteFilterDescription->setChecked(false);

    m_ui->sortFavourites->setChecked(false);

    emit changed(true);
}

/**
 * Filters emojis based on filters and the search term
 */
void EmojiRunnerConfig::filterEmojiListView() {
    if (!filterActive) return;

    const QString text = m_ui->favouriteFilter->text();
    const int count = m_ui->emojiListView->count();

    if (text.isEmpty()) {
        unhideAll();
    } else {
        bool filterName = favouriteFilters.contains("name");
        bool filterDescription = favouriteFilters.contains("description");
        bool filterTags = favouriteFilters.contains("tags");
        bool custom = favouriteFilters.contains("custom");
        for (int i = 0; i < count; ++i) {
            auto *item = m_ui->emojiListView->item(i);
            const auto emoji = allEmojis.value(item->data(1).toString());
            bool hidden = true;

            // If the category is disabled and it is not a favourite it should always be hidden
            if (item->checkState() != Qt::Checked && disabledEmojiCategoryNames.contains(emoji.category)) {
                item->setHidden(true);
                continue;
            }

            // If custom is selected only the custom entries are filterd by the criterias
            if (custom && emoji.category != "Custom") {
                item->setHidden(true);
                continue;
            }

            if (filterName) {
                if (emoji.displayName.contains(text, Qt::CaseInsensitive) || emoji.name.contains(text, Qt::CaseInsensitive)) {
                    hidden = false;
                }
            }
            if (hidden && filterDescription) {
                if (emoji.description.contains(text, Qt::CaseInsensitive)) hidden = false;
            }
            if (hidden && filterTags) {
                for (const auto &t:emoji.tags) if (t.contains(text, Qt::CaseInsensitive)) hidden = false;
            }
            if (emoji.matchesVersions(configUnicodeVersion, configIosVersion) || item->checkState() == Qt::Checked) {
                item->setHidden(hidden);
            } else item->setHidden(true);
        }
    }

    displayVisibleItems();
}

/**
 * Enable/Disable filter checkboxes and search with new filters
 * @param reloadFilter
 */
void EmojiRunnerConfig::filtersChanged(bool reloadFilter) {
    // Enable/Disable the filter checkboxes and trigger search event
    int checked = 0;
    favouriteFilters.clear();
    if (m_ui->favouriteFilterName->isChecked()) {
        checked++;
        favouriteFilters.append("name");
    }
    if (m_ui->favouriteFilterTags->isChecked()) {
        checked++;
        favouriteFilters.append("tags");
    }
    if (m_ui->favouriteFilterDescription->isChecked()) {
        checked++;
        favouriteFilters.append("description");
    }
    bool customFilterChanged = false;
    if (m_ui->favouriteFilterCustom->isChecked()) {
        favouriteFilters.append("custom");
        if (!customFilterChecked) customFilterChanged = true;
        customFilterChecked = true;
    } else {
        if (customFilterChecked) {
            customFilterChanged = true;
        }
        customFilterChecked = false;
    }

    if (checked == 1) {
        if (m_ui->favouriteFilterName->isChecked()) m_ui->favouriteFilterName->setDisabled(true);
        else if (m_ui->favouriteFilterTags->isChecked()) m_ui->favouriteFilterTags->setDisabled(true);
        else m_ui->favouriteFilterDescription->setDisabled(true);
    } else {
        m_ui->favouriteFilterName->setDisabled(false);
        m_ui->favouriteFilterTags->setDisabled(false);
        m_ui->favouriteFilterDescription->setDisabled(false);
    }

    if (reloadFilter && (!m_ui->favouriteFilter->text().isEmpty() || customFilterChanged)) filterEmojiListView();
}

/**
 * Check for newly enabled/disabled categories and add/remove the emojis
 */
void EmojiRunnerConfig::categoriesChanged() {
    const QStringList previouslyDisabled = disabledEmojiCategoryNames;
    disabledEmojiCategoryNames.clear();

    // Update list of disabled categories
    const int count = m_ui->categoryListView->count();
    for (int i = 0; i < count; ++i) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) disabledEmojiCategoryNames.append(item->text());
    }
    filterEmojiListView();
}

/**
 * Toggle if only checked items are shown
 */
void EmojiRunnerConfig::showOnlyFavourites() {
    const bool checked = m_ui->sortFavourites->isChecked();
    const int itemCount = m_ui->emojiListView->count();

    m_ui->favouriteFilter->clear();
    m_ui->favouriteFilter->setDisabled(checked);
    m_ui->favouriteFilterName->setDisabled(checked);
    m_ui->favouriteFilterTags->setDisabled(checked);
    m_ui->favouriteFilterDescription->setDisabled(checked);


    if (checked) {
        filterActive = false;
        for (int i = 0; i < itemCount; ++i) {
            auto *item = m_ui->emojiListView->item(i);
            item->setHidden(item->checkState() == Qt::Unchecked);
        }
    } else {
        unhideAll();
        filterActive = true;
    }

    displayVisibleItems();
}

/**
 * Update configUnicodeVersion variable, clear filter text and call filterFavourites()
 */
void EmojiRunnerConfig::unicodeVersionChanged() {
    configUnicodeVersion = m_ui->unicodeComboBox->currentText().toFloat();
    m_ui->favouriteFilter->clear();
    filterEmojiListView();
}

/**
 * Update configIosVersion variable, clear filter text and call filterFavourites()
 */
void EmojiRunnerConfig::iosVersionChanged() {
    configIosVersion = m_ui->iosComboBox->currentText().toFloat();
    m_ui->favouriteFilter->clear();
    filterEmojiListView();
}

/**
 * Check if there is another favourite above/below the selected and en-/disable the corresponding button
 */
void EmojiRunnerConfig::validateMoveFavouriteButtons() {
    const auto *currentItem = m_ui->emojiListView->currentItem();
    if (m_ui->sortFavourites->isChecked() || (currentItem != nullptr && currentItem->checkState() == Qt::Checked)) {
        const int rowCount = m_ui->emojiListView->count() - 1;
        const int currentRow = m_ui->emojiListView->currentRow();

        // Handle Up Button
        bool hasNoFavouriteAbove = true;
        for (int i = 0; i < currentRow; i++) {
            if (m_ui->emojiListView->item(i)->checkState() == Qt::Checked) {
                hasNoFavouriteAbove = false;
                break;
            }
        }
        m_ui->moveFavouriteUp->setDisabled(hasNoFavouriteAbove);

        // Handle Down Button
        bool hasNoFavouriteBelow = true;
        for (int i = rowCount; i > currentRow; i--) {
            if (m_ui->emojiListView->item(i)->checkState() == Qt::Checked) {
                hasNoFavouriteBelow = false;
                break;
            }
        }
        m_ui->moveFavouriteDown->setDisabled(hasNoFavouriteBelow);
    } else {
        m_ui->moveFavouriteUp->setDisabled(true);
        m_ui->moveFavouriteDown->setDisabled(true);
    }

}

/**
 * Change index with one of favourite that is below
 */
void EmojiRunnerConfig::moveFavouriteUp() {
    const int currentRow = m_ui->emojiListView->currentRow();
    int aboveIndex = -1;

    // Change index with one of favourite that is above
    for (int i = currentRow - 1; i >= 0; --i) {
        if (m_ui->emojiListView->item(i)->checkState() == Qt::Checked) {
            aboveIndex = i;
            break;
        }
    }
    // If for example an emoji has been favourized which is at the end of the list, it gets moved right below
    // the favourite which is above, otherwise the emoji would be put above and this would be confusing if the user
    // could not see the favourite above because of the emojis in between
    if (aboveIndex > 0) {
        const auto *item = m_ui->emojiListView->item(aboveIndex + 1);
        if (!item->isHidden() && item->checkState() == Qt::Unchecked) aboveIndex++;
    }
    auto *item = m_ui->emojiListView->takeItem(currentRow);
    m_ui->emojiListView->insertItem(aboveIndex, item);
    m_ui->emojiListView->setCurrentRow(aboveIndex);
}

/**
 * Change index with one of favourite that is above
 */
void EmojiRunnerConfig::moveFavouriteDown() {
    const int count = m_ui->emojiListView->count();
    const int currentRow = m_ui->emojiListView->currentRow();
    int belowIndex = true;

    for (int i = currentRow + 1; i < count; ++i) {
        if (m_ui->emojiListView->item(i)->checkState() == Qt::Checked) {
            belowIndex = i;
            break;
        }
    }
    auto *item = m_ui->emojiListView->takeItem(currentRow);
    m_ui->emojiListView->insertItem(belowIndex, item);
    m_ui->emojiListView->setCurrentRow(belowIndex);
}

/**
 * Show all items if they mathe the unicode version or if they are checked
 */
void EmojiRunnerConfig::unhideAll() {
    const int itemCount = m_ui->emojiListView->count();
    bool custom = favouriteFilters.contains("custom");

    for (int i = 0; i < itemCount; ++i) {
        auto *item = m_ui->emojiListView->item(i);
        const auto emoji = allEmojis.value(item->data(1).toString());
        item->setHidden(
                ((
                         !emoji.matchesVersions(configUnicodeVersion, configIosVersion) ||
                         disabledEmojiCategoryNames.contains(emoji.category)
                 )
                 && item->checkState() == Qt::Unchecked) ||
                (custom && emoji.category != "Custom")
        );
    }
}

/**
 * Count visible items and update the value in the UI
 */
void EmojiRunnerConfig::displayVisibleItems() const {
    int visibleItems = 0;
    int count = this->m_ui->emojiListView->count();
    for (int i = 0; i < count; ++i) {
        const auto *item = this->m_ui->emojiListView->item(i);
        if (!item->isHidden()) ++visibleItems;
    }

    m_ui->favouriteVisibleElements->setText(QString::number(visibleItems) + " Elements");
}

/**
 * Checks if the number of favourites is greater than 20, if true it shows the maxFavouritesLabel with a warning
 */
void EmojiRunnerConfig::checkMaxFavourites() {
    int favourites = 0;
    int count = this->m_ui->emojiListView->count();
    for (int i = 0; i < count; ++i) {
        if (this->m_ui->emojiListView->item(i)->checkState() == Qt::Checked) favourites++;
    }

    if (favourites > 20) {
        m_ui->maxFavouritesLabel->setText("You can not select more than 20 Favourites!");
        m_ui->maxFavouritesLabel->setHidden(false);
    } else {
        m_ui->maxFavouritesLabel->setHidden(true);
    }
}

/*
 * Change the font size based on the value
 */
void EmojiRunnerConfig::changeFontSize(int value) {
    auto f = QFont(m_ui->emojiListView->font());
    f.setPixelSize(value / 2);
    m_ui->emojiListView->setFont(f);
}

/**
 * Show popup to add a new emoji
 */
void EmojiRunnerConfig::addEmoji() {
    auto *popup = new EmojiRunnerPopup(this);
    popup->show();
    connect(popup, SIGNAL(finished(Emoji, QString)), this, SLOT(applyEmojiPopupResults(Emoji, QString)));
}

/**
 * Show popup to edit an emoji
 */
void EmojiRunnerConfig::editEmoji() {
    const auto *item = m_ui->emojiListView->currentItem();
    if (item != nullptr) {
        auto *popup = new EmojiRunnerPopup(this, allEmojis.value(item->data(1).toString()));
        popup->show();
        connect(popup, SIGNAL(finished(Emoji, QString)), this, SLOT(applyEmojiPopupResults(Emoji, QString)));
    }
}

/**
 * Applies the results of the popup to the emojiListView
 *
 * @param emoji
 * @param originalName
 */
void EmojiRunnerConfig::applyEmojiPopupResults(const Emoji &emoji, const QString &originalName) {
    if (!allEmojis.contains(emoji.name) && originalName.isEmpty()) {
        m_ui->emojiListView->insertItem(0, emoji.toListWidgetItem());
        if (!customEntriesExist) {
            customEntriesExist = true;
            auto *item = new QListWidgetItem();
            item->setText("Custom");
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            m_ui->categoryListView->addItem(item);
        }
    } else {
        const auto items = m_ui->emojiListView->findItems(" " + originalName, Qt::MatchEndsWith);
        if (items.count() == 1) {
            auto *item = items.at(0);
            const int row = m_ui->emojiListView->row(item);
            delete m_ui->emojiListView->takeItem(row);
            m_ui->emojiListView->insertItem(row, emoji.toListWidgetItem());
        } else {
            m_ui->emojiListView->insertItem(0, emoji.toListWidgetItem());
            qInfo() << "Could not update" << emoji.name << originalName;
        }
    }
    allEmojis.insert(emoji.name, emoji);
}

/**
 * Deletes emoji from ListView
 */
void EmojiRunnerConfig::deleteEmoji() {
    QMessageBox::StandardButton reply = QMessageBox::
    question(this, "Confirm Delete", "Do you want to delete this custom emoji ?",
             QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) delete m_ui->emojiListView->takeItem(m_ui->emojiListView->currentRow());
}

/**
 * Enable/Disable the edit and delete buttons based on the current emoji of the emojiListView
 */
void EmojiRunnerConfig::validateEditingOptions() {
    const auto *item = m_ui->emojiListView->currentItem();
    const bool disabled = item == nullptr || allEmojis.value(item->data(1).toString()).category != "Custom";
    m_ui->editEmojiPushButton->setDisabled(disabled);
    m_ui->deleteEmojiPushButton->setDisabled(disabled);
}


#include "emojirunner_config.moc"

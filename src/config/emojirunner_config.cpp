#include "emojirunner_config.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <FileReader.h>
#include <QDebug>

K_PLUGIN_FACTORY(EmojiRunnerConfigFactory, registerPlugin<EmojiRunnerConfig>("kcm_krunner_emojirunner");)

EmojiRunnerConfigForm::EmojiRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

EmojiRunnerConfig::EmojiRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new EmojiRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    emojiCategories = FileReader::readJSONFile(true);
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");
    disabledEmojis = config.readEntry("disabledCategories", "").split(";", QString::SplitBehavior::SkipEmptyParts);

    std::sort(emojiCategories.begin(), emojiCategories.end(), [](const EmojiCategory &c1, const EmojiCategory &c2) -> bool {
        if (c1.name == "Smileys & Emotion") return true;
        return c1.name < c2.name;
    });
    // Connect slots for filters
    connect(m_ui->favouriteListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(changed()));
    connect(m_ui->enableGlobalSearch, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->singleRunnerModePaste, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->favouriteFilter, SIGNAL(textChanged(QString)), this, SLOT(filterFavourites()));
    connect(m_ui->favouriteFilterName, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterDescription, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterTags, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterDescription_2, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->favouriteFilterTags_2, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Unicode Versions change => eventually reload filters
    connect(m_ui->unicodeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unicodeVersionsChanged()));
    connect(m_ui->unicodeComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(changed()));
    connect(m_ui->iosComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(iosVersionsChanged()));
    connect(m_ui->iosComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(changed()));
    // Disable categories
    connect(m_ui->categoryListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(changed()));
    connect(m_ui->categoryListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(categoriesApplyChanges()));
    // Sort favourites
    connect(m_ui->sortFavourites, SIGNAL(clicked(bool)), this, SLOT(showOnlyFavourites()));
    connect(m_ui->sortFavourites, SIGNAL(clicked(bool)), this, SLOT(validateMoveFavouriteButtons()));
    connect(m_ui->favouriteListView, SIGNAL(itemSelectionChanged()), this, SLOT(validateMoveFavouriteButtons()));
    // Move favourites up/down
    connect(m_ui->moveFavouriteUp, SIGNAL(clicked(bool)), this, SLOT(moveFavouriteUp()));
    connect(m_ui->moveFavouriteUp, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->moveFavouriteDown, SIGNAL(clicked(bool)), this, SLOT(moveFavouriteDown()));
    connect(m_ui->moveFavouriteDown, SIGNAL(clicked(bool)), this, SLOT(changed()));
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
        disabledEmojis.contains(category.name) ? item->setCheckState(Qt::Unchecked) : item->setCheckState(Qt::Checked);
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
        m_ui->favouriteListView->addItem(emoji.toListWidgetItem());
    }

    // Load other emojis
    allEmojis.clear();
    QStringList disabledCategories = config.readEntry("disabledCategories").split(";", QString::SplitBehavior::SkipEmptyParts);
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") continue;
        for (const auto &emoji:category.emojis.values()) {
            allEmojis.insert(emoji.name, emoji);
            if (favouriteNames.contains(emoji.name)) continue;
            m_ui->favouriteListView->addItem(emoji.toListWidgetItem());
        }
    }

    // Load versions
    for (const auto &unicodeVersion:unicodeVersions) {
        m_ui->unicodeComboBox->addItem(QString::number(unicodeVersion));
    }
    for (const auto &iosVersion:iosVersions) {
        m_ui->iosComboBox->addItem(QString::number(iosVersion));
    }
    configUnicodeVersion = config.readEntry("unicodeVersion", "11").toFloat();
    configIosVersion = config.readEntry("iosVersion", "13").toFloat();
    m_ui->unicodeComboBox->setCurrentText(QString::number(configUnicodeVersion));
    m_ui->iosComboBox->setCurrentText(QString::number(configIosVersion));

    categoriesApplyChanges();
    filterActive = true;
    filterFavourites();
    validateMoveFavouriteButtons();

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

    // Save categories
    QString disabledCategories;
    const int categoryCount = m_ui->categoryListView->count();
    for (int i = 0; i < categoryCount; i++) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) disabledCategories.append(item->text() + ";");
    }
    config.writeEntry("disabledCategories", disabledCategories);

    // Save favourites
    const int favouriteCount = m_ui->favouriteListView->count();
    QString favouriteIDs;
    for (int i = 0; i < favouriteCount; i++) {
        const auto *item = m_ui->favouriteListView->item(i);
        if (item->checkState() == Qt::Checked) {
            const auto emoji = allEmojis.value(item->data(1).toString());
            favouriteIDs.append(QString::number(emoji.id) + ";");
        }
    }
    config.writeEntry("favourites", favouriteIDs);

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
 * Filter emojis based on filters and the search term
 */
void EmojiRunnerConfig::filterFavourites() {
    if (!filterActive) return;

    const QString text = m_ui->favouriteFilter->text();
    int count = m_ui->favouriteListView->count();
    configUnicodeVersion = m_ui->unicodeComboBox->currentText().toFloat();
    configIosVersion = m_ui->iosComboBox->currentText().toFloat();


    if (text.isEmpty()) {
        unhideAll();
    } else {
        bool filterName = favouriteFilters.contains("name");
        bool filterDescription = favouriteFilters.contains("description");
        bool filterTags = favouriteFilters.contains("tags");
        for (int i = 0; i < count; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            const auto emoji = allEmojis.value(item->data(1).toString());
            bool hidden = true;

            if (filterName) {
                if (emoji.name.contains(text, Qt::CaseInsensitive) ||
                    emoji.name.contains(QString(text).replace(' ', '_'), Qt::CaseInsensitive)) {
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

    if (checked == 1) {
        if (m_ui->favouriteFilterName->isChecked()) m_ui->favouriteFilterName->setDisabled(true);
        else if (m_ui->favouriteFilterTags->isChecked()) m_ui->favouriteFilterTags->setDisabled(true);
        else m_ui->favouriteFilterDescription->setDisabled(true);
    } else {
        m_ui->favouriteFilterName->setDisabled(false);
        m_ui->favouriteFilterTags->setDisabled(false);
        m_ui->favouriteFilterDescription->setDisabled(false);
    }

    if (reloadFilter && !m_ui->favouriteFilter->text().isEmpty()) filterFavourites();
}

/**
 * Check for newly enabled/disabled categories and add/remove the emojis
 */
void EmojiRunnerConfig::categoriesApplyChanges() {
    const QStringList previouslyDisabled = disabledEmojis;
    disabledEmojis.clear();

    // Update list of disabled categories
    const int count = m_ui->categoryListView->count();
    for (int i = 0; i < count; ++i) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) disabledEmojis.append(item->text());
    }

    // Remove items of disabled categories from list
    const int emojiItems = m_ui->favouriteListView->count();
    QList<int> remove;
    if (!disabledEmojis.isEmpty()) {
        for (int i = 0; i < emojiItems; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            const auto emoji = allEmojis.value(item->data(1).toString());
            if (!disabledEmojis.contains(emoji.category)) continue;
            if (item->checkState() == Qt::Checked) continue;
            remove.insert(0, i);
        }
    }
    for (int rm:remove) m_ui->favouriteListView->model()->removeRow(rm);

    // Get newly enabled categories
    QStringList newlyEnabled;
    for (const auto &c:previouslyDisabled) {
        if (!disabledEmojis.contains(c)) newlyEnabled.append(c);
    }
    // Add newly installed emojis
    if (!newlyEnabled.isEmpty()) {
        // Check which favourite emojis are in list ( because they are favourites)
        QStringList newFavourites;
        const int newItemCount = m_ui->favouriteListView->count();
        for (int i = 0; i < newItemCount; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            const auto emoji = allEmojis.value(item->data(1).toString());
            if (!disabledEmojis.contains(emoji.category)) newFavourites.append(emoji.name);
        }
        // Add emojis, skip favourites
        for (const auto &c:emojiCategories) {
            if (!newlyEnabled.contains(c.name)) continue;
            for (const auto &emoji:c.emojis.values()) {
                const QString text = emoji.emoji + " " + QString(emoji.name).replace('_', ' ');

                if (newFavourites.contains(emoji.name)) continue;

                auto *item = emoji.toListWidgetItem();
                m_ui->favouriteListView->addItem(item);
                item->setHidden(!emoji.matchesVersions(configUnicodeVersion, configIosVersion));
            }
        }
    }

    if (!m_ui->favouriteFilter->text().isEmpty()) {
        const int newItemCount = m_ui->favouriteListView->count();
        for (int i = 0; i < newItemCount; i++) {
            const auto item = m_ui->favouriteListView->item(i);
            item->setHidden(false);
        }
        m_ui->favouriteFilter->clear();
    }
    displayVisibleItems();
}

/**
 * Toggle if only checked items are shown
 */
void EmojiRunnerConfig::showOnlyFavourites() {
    const bool checked = m_ui->sortFavourites->isChecked();
    const int itemCount = m_ui->favouriteListView->count();

    m_ui->favouriteFilter->clear();
    m_ui->favouriteFilter->setDisabled(checked);
    m_ui->favouriteFilterName->setDisabled(checked);
    m_ui->favouriteFilterTags->setDisabled(checked);
    m_ui->favouriteFilterDescription->setDisabled(checked);


    if (checked) {
        filterActive = false;
        for (int i = 0; i < itemCount; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            if (item->checkState() == Qt::Unchecked) item->setHidden(true);
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
void EmojiRunnerConfig::unicodeVersionsChanged() {
    configUnicodeVersion = m_ui->unicodeComboBox->currentText().toFloat();
    m_ui->favouriteFilter->clear();
    filterFavourites();
}

/**
 * Update configIosVersion variable, clear filter text and call filterFavourites()
 */
void EmojiRunnerConfig::iosVersionsChanged() {
    configIosVersion = m_ui->iosComboBox->currentText().toFloat();
    m_ui->favouriteFilter->clear();
    filterFavourites();
}

/**
 * Check if there is another favourite above/below the selected and en-/disable the corresponding button
 */
void EmojiRunnerConfig::validateMoveFavouriteButtons() {

    if (m_ui->sortFavourites->isChecked()) {
        const int rowCount = m_ui->favouriteListView->count() - 1;
        const int currentRow = m_ui->favouriteListView->currentRow();

        // Handle Up Button
        bool hasNoFavouriteAbove = true;
        for (int i = 0; i < currentRow; i++) {
            if (!m_ui->favouriteListView->item(i)->isHidden()) {
                hasNoFavouriteAbove = false;
                break;
            }
        }
        m_ui->moveFavouriteUp->setDisabled(hasNoFavouriteAbove);

        // Handle Down Button
        bool hasNoFavouriteBelow = true;
        for (int i = rowCount; i > currentRow; i--) {
            if (!m_ui->favouriteListView->item(i)->isHidden()) {
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
    const int currentRow = m_ui->favouriteListView->currentRow();
    int aboveIndex = true;

    // Change index with one of favourite that is above
    for (int i = currentRow - 1; i >= 0; --i) {
        if (!m_ui->favouriteListView->item(i)->isHidden()) {
            aboveIndex = i;
            break;
        }
    }
    auto *item = m_ui->favouriteListView->takeItem(currentRow);
    m_ui->favouriteListView->insertItem(aboveIndex, item);
    m_ui->favouriteListView->setCurrentRow(aboveIndex);
}

/**
 * Change index with one of favourite that is above
 */
void EmojiRunnerConfig::moveFavouriteDown() {
    const int count = m_ui->favouriteListView->count();
    const int currentRow = m_ui->favouriteListView->currentRow();
    int belowIndex = true;

    for (int i = currentRow + 1; i < count; ++i) {
        if (!m_ui->favouriteListView->item(i)->isHidden()) {
            belowIndex = i;
            break;
        }
    }
    auto *item = m_ui->favouriteListView->takeItem(currentRow);
    m_ui->favouriteListView->insertItem(belowIndex, item);
    m_ui->favouriteListView->setCurrentRow(belowIndex);
}

/**
 * Show all items if they mathe the unicode version or if they are checked
 */
void EmojiRunnerConfig::unhideAll() {
    const int itemCount = m_ui->favouriteListView->count();
    for (int i = 0; i < itemCount; ++i) {
        auto *item = m_ui->favouriteListView->item(i);
        const auto emoji = allEmojis.value(item->data(1).toString());
        item->setHidden(!emoji.matchesVersions(configUnicodeVersion, configIosVersion) && item->checkState() == Qt::Unchecked);
    }
}

/**
 * Count visible items and update the value in the UI
 */
void EmojiRunnerConfig::displayVisibleItems() const {
    int visibleItems = 0;
    int count = this->m_ui->favouriteListView->count();
    for (int i = 0; i < count; ++i) {
        if (!this->m_ui->favouriteListView->item(i)->isHidden()) ++visibleItems;
    }

    m_ui->favouriteVisibleElements->setText(QString::number(visibleItems) + " Elements");
}

#include "emojirunner_config.moc"

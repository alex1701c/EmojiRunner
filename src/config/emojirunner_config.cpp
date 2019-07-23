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

// TODO Implement Mode Favourites Functionality
// TODO Handle Save
// TODO Apply Settings directly in file read function to avoid unnecessary items


EmojiRunnerConfig::EmojiRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new EmojiRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    emojiCategories = FileReader::readJSONFile();
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");
    disabledEmojis = config.readEntry("disabledCategories", "").split(";", QString::SplitBehavior::SkipEmptyParts);

    std::sort(emojiCategories.begin(), emojiCategories.end(), [](EmojiCategory c1, EmojiCategory c2) -> bool {
        if (c1.name == "Smileys & Emotion") return true;
        return c1.name < c2.name;
    });

    // Connect slots for filters
    connect(m_ui->favouriteFilter, SIGNAL(textChanged(QString)), this, SLOT(filterFavourites()));
    connect(m_ui->favouriteFilterName, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterDescription, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterTags, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    // Unicode Versions change => eventually reload filters
    connect(m_ui->unicodeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unicodeVersionsChanged()));
    connect(m_ui->unicodeComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(changed()));
    connect(m_ui->iosComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(iosVersionsChanged()));
    connect(m_ui->iosComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(changed()));
    // Disable categories
    connect(m_ui->applyCategoryChanges, SIGNAL(clicked(bool)), this, SLOT(categoriesApplyChanges()));
    connect(m_ui->applyCategoryChanges, SIGNAL(clicked(bool)), this, SLOT(changed()));
    // Sort favourites
    connect(m_ui->sortFavourites, SIGNAL(clicked(bool)), this, SLOT(showOnlyFavourites()));
    connect(m_ui->favouriteListView, SIGNAL(itemSelectionChanged()), this, SLOT(validateMoveFavouriteButtons()));
    connect(m_ui->favouriteChangesSaveButton, SIGNAL(clicked(bool)), this, SLOT(saveFavourites()));
    connect(m_ui->favouriteChangesSaveButton, SIGNAL(clicked(bool)), this, SLOT(changed()));

    connect(m_ui->moveFavouriteUp, SIGNAL(clicked(bool)), this, SLOT(moveFavouriteUp()));
    connect(m_ui->moveFavouriteUp, SIGNAL(clicked(bool)), this, SLOT(changed()));
    connect(m_ui->moveFavouriteDown, SIGNAL(clicked(bool)), this, SLOT(moveFavouriteDown()));
    connect(m_ui->moveFavouriteDown, SIGNAL(clicked(bool)), this, SLOT(changed()));
}

void EmojiRunnerConfig::load() {
    m_ui->enableGlobalSearch->setChecked(config.readEntry("globalSearch", "true") == "true");

    // Load categories
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") continue;
        auto *item = new QListWidgetItem();
        item->setText(category.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        disabledEmojis.contains(category.name) ? item->setCheckState(Qt::Unchecked) : item->setCheckState(Qt::Checked);
        m_ui->categoryListView->addItem(item);
    }

    // Load favourites at top
    QStringList favouriteNames;
    for (const auto &category:emojiCategories) {
        if (category.name != "Favourites") continue;
        favouriteNames = category.emojis.keys();
        for (const auto &emoji:category.emojis.values()) {
            auto *item = new QListWidgetItem(emoji.emoji + " " + QString(emoji.name).replace('_', ' '));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            item->setData(1, emoji.name);
            m_ui->favouriteListView->addItem(item);
        }
    }
    // Load other emojis
    allEmojis.clear();
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") continue;
        for (const auto &emoji:category.emojis.values()) {
            allEmojis.insert(emoji.name, emoji);
            if (favouriteNames.contains(emoji.name)) continue;
            auto *item = new QListWidgetItem(emoji.emoji + " " + QString(emoji.name).replace('_', ' '));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            item->setData(1, emoji.name);
            m_ui->favouriteListView->addItem(item);
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

    m_ui->favouriteListView->currentRowChanged(0);

    filterActive = true;
    filterFavourites();
    validateMoveFavouriteButtons();

    emit changed(true);
}


void EmojiRunnerConfig::save() {
    config.writeEntry("globalSearch", m_ui->enableGlobalSearch->isChecked() ? "true" : "false");
    config.writeEntry("unicodeVersion", m_ui->unicodeComboBox->currentText());
    config.writeEntry("iosVersion", m_ui->iosComboBox->currentText());

    QString disabledCategories;
    const int categoryCount = m_ui->categoryListView->count();
    for (int i = 0; i < categoryCount; i++) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) disabledCategories.append(item->text() + ";");
    }
    config.writeEntry("disabledCategories", disabledCategories);

    saveFavourites();
    emit changed();
}

void EmojiRunnerConfig::defaults() {

    m_ui->enableGlobalSearch->setChecked(true);

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

void EmojiRunnerConfig::filterFavourites() {
    if (!filterActive) return;

    const QString text = m_ui->favouriteFilter->text();
    int count = m_ui->favouriteListView->count();

    if (text.isEmpty()) {
        // Show all entries if unicode matches
        for (int i = 0; i < count; i++) {
            auto *item = m_ui->favouriteListView->item(i);
            const auto emoji = allEmojis.value(item->data(1).toString());
            m_ui->favouriteListView->item(i)->setHidden(
                    !emoji.matchesVersions(configUnicodeVersion, configIosVersion) && item->checkState() == Qt::Unchecked
            );
        }
    } else {
        for (int i = 0; i < count; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            const auto emoji = allEmojis.value(item->data(1).toString());
            bool hidden = true;

            // Skip if unicode/ios version is to great
            // Search properties based on selected filters
            if (favouriteFilters.contains("name")) {
                if (emoji.name.contains(text, Qt::CaseInsensitive) ||
                    emoji.name.contains(QString(text).replace(' ', '_'), Qt::CaseInsensitive)) {
                    hidden = false;
                }
            }
            if (hidden && favouriteFilters.contains("description")) {
                if (emoji.description.contains(text, Qt::CaseInsensitive))hidden = false;
            }
            if (hidden && favouriteFilters.contains("tags")) {
                for (const auto &t:emoji.tags)if (t.contains(text, Qt::CaseInsensitive)) hidden = false;
            }
            if (emoji.matchesVersions(configUnicodeVersion, configIosVersion) || item->checkState() == Qt::Checked) {
                item->setHidden(hidden);
            } else item->setHidden(true);
        }
    }

    displayVisibleItems();
}

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

    // Get newly enabled items
    QStringList newlyEnabled;
    for (const auto &c:previouslyDisabled) {
        if (!disabledEmojis.contains(c)) newlyEnabled.append(c);
    }
    // Add newly installed emojis
    if (!newlyEnabled.isEmpty()) {
        // Check which favourite emojis are in list
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

                auto *item = new QListWidgetItem(text);
                item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                item->setCheckState(Qt::Unchecked);
                item->setData(1, emoji.name);
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

void EmojiRunnerConfig::showOnlyFavourites() {
    const bool checked = m_ui->sortFavourites->isChecked();
    const int itemCount = m_ui->favouriteListView->count();

    m_ui->favouriteFilter->clear();
    m_ui->applyCategoryChanges->setDisabled(checked);
    m_ui->favouriteFilter->setDisabled(checked);
    m_ui->favouriteFilterName->setDisabled(checked);
    m_ui->favouriteFilterTags->setDisabled(checked);
    m_ui->favouriteFilterDescription->setDisabled(checked);


    if (checked) {
        filterActive = false;
        favouriteVisibleEmojis = -1;
        for (int i = 0; i < itemCount; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            if (item->checkState() == Qt::Unchecked) item->setHidden(true);
            else ++favouriteVisibleEmojis;
        }
    } else {
        for (int i = 0; i < itemCount; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            const auto emoji = allEmojis.value(item->data(1).toString());
            item->setHidden(!emoji.matchesVersions(configUnicodeVersion, configIosVersion) && item->checkState() == Qt::Unchecked);
        }
        filterActive = true;
    }

    displayVisibleItems();
}

void EmojiRunnerConfig::unicodeVersionsChanged() {
    configUnicodeVersion = m_ui->unicodeComboBox->currentText().toFloat();
    if (m_ui->favouriteFilter->text().isEmpty()) {
        filterFavourites();
    } else {
        m_ui->favouriteFilter->clear();
    }
}

void EmojiRunnerConfig::iosVersionsChanged() {
    configIosVersion = m_ui->iosComboBox->currentText().toFloat();
    if (m_ui->favouriteFilter->text().isEmpty()) {
        filterFavourites();
    } else {
        m_ui->favouriteFilter->clear();
    }
}


void EmojiRunnerConfig::validateMoveFavouriteButtons() {
    if (m_ui->sortFavourites->isChecked()) {
        m_ui->moveFavouriteUp->setDisabled(m_ui->favouriteListView->currentRow() == 0);
        m_ui->moveFavouriteDown->setDisabled(m_ui->favouriteListView->currentRow() == favouriteVisibleEmojis);
    } else {
        m_ui->moveFavouriteUp->setDisabled(true);
        m_ui->moveFavouriteDown->setDisabled(true);
    }

}


void EmojiRunnerConfig::moveFavouriteUp() {
    const int row = m_ui->favouriteListView->currentRow();
    auto *item = m_ui->favouriteListView->takeItem(row);
    m_ui->favouriteListView->insertItem(row - 1, item);
    m_ui->favouriteListView->setCurrentRow(row - 1);
}

void EmojiRunnerConfig::moveFavouriteDown() {
    const int row = m_ui->favouriteListView->currentRow();
    auto *item = m_ui->favouriteListView->takeItem(row);
    m_ui->favouriteListView->insertItem(row + 1, item);
    m_ui->favouriteListView->setCurrentRow(row + 1);
}

void EmojiRunnerConfig::saveFavourites() {
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
}


#include "emojirunner_config.moc"

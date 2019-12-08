#include "emojirunner_config.h"
#include "emojirunner_popup.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <krunner/abstractrunner.h>
#include <core/FileReader.h>
#include <QDebug>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QDialog>
#include <QtWidgets/QInputDialog>
#include <QJsonObject>
#include <QJsonArray>
#include "utilities.h"
#include <core/Config.h>

K_PLUGIN_FACTORY(EmojiRunnerConfigFactory, registerPlugin<EmojiRunnerConfig>("kcm_krunner_emojirunner");)

EmojiRunnerConfigForm::EmojiRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

/**
 * TODO Fix bug that custom emojis are not loaded !!!
 */

EmojiRunnerConfig::EmojiRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new EmojiRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    config = KSharedConfig::openConfig(QDir::homePath() + "/.config/krunnerplugins/" + Config::ConfigFileName)
            ->group(Config::RootGroup);
    config.config()->reparseConfiguration();

    FileReader reader(config);
    emojiCategories = reader.getEmojiCategories(true);
    disabledEmojiCategoryNames = reader.disabledCategories;

    std::sort(emojiCategories.begin(), emojiCategories.end(), [](const EmojiCategory &c1, const EmojiCategory &c2) -> bool {
        if (c1.name == Config::Config::CustomCategory) return true;
        if (c1.name == Config::SmileysEmotionsCategory && c2.name != Config::CustomCategory) return true;
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
    connect(m_ui->emojiListView, SIGNAL(itemChanged(QListWidgetItem * )), SLOT(checkMaxFavourites()));
    // For Drag/Drop events
    connect(m_ui->emojiListView, SIGNAL(currentRowChanged(int)), SLOT(changed()));
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
    // Toggle favourite search options
    connect(m_ui->toggleFavouritesPushButton, SIGNAL(clicked(bool)), this, SLOT(toggleFavouriteOptions()));
}

void EmojiRunnerConfig::load() {
    m_ui->enableGlobalSearch->setChecked(config.readEntry(Config::GlobalSearch, true));
    m_ui->singleRunnerModePaste->setChecked(config.readEntry(Config::SingleRunnerModePaste, true));
    m_ui->favouriteFilterDescription_2->setChecked(config.readEntry(Config::SearchByDescription, false));
    m_ui->favouriteFilterTags_2->setChecked(config.readEntry(Config::SearchByTags, false));

    // Load categories
    for (const auto &category:emojiCategories) {
        if (category.name == Config::FavouritesCategory) continue;
        auto *item = new QListWidgetItem();
        item->setText(category.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (category.name == Config::CustomCategory) customEntriesExist = true;
        item->setCheckState(disabledEmojiCategoryNames.contains(category.name) ? Qt::Unchecked : Qt::Checked);
        m_ui->categoryListView->addItem(item);
    }

    // Load favourites at top, apply sort from config
    QStringList favouriteNames;
    QList<Emoji> favouriteEmojisToAdd;
    for (const auto &category:emojiCategories) {
        if (category.name != Config::FavouritesCategory) continue;
        favouriteNames = category.emojis.keys();
        for (const auto &emoji:category.emojis.values()) {
            favouriteEmojisToAdd.append(emoji);
        }
    }
    std::sort(favouriteEmojisToAdd.begin(), favouriteEmojisToAdd.end(), [](const Emoji &e1, const Emoji &e2) -> bool {
        return e1.favourite > e2.favourite;
    });
    for (const auto &emoji:favouriteEmojisToAdd) {
        m_ui->emojiListView->addItem(Utilities::toListWidgetItem(emoji));
    }

    // Load other emojis
    allEmojis.clear();
    for (const auto &category:emojiCategories) {
        if (category.name == Config::FavouritesCategory) continue;
        for (const auto &emoji:category.emojis.values()) {
            allEmojis.insert(emoji.name, emoji);
            if (favouriteNames.contains(emoji.name)) continue;
            auto *item = Utilities::toListWidgetItem(emoji);
            m_ui->emojiListView->addItem(item);
        }
    }

    // Load versions
    m_ui->unicodeComboBox->addItems(QString(Config::UnicodeVersionChoices).split(','));
    m_ui->iosComboBox->addItems(QString(Config::IosVersionChoices).split(','));
    configUnicodeVersion = config.readEntry(Config::UnicodeVersion, QVariant(Config::DefaultUnicodeVersion).toFloat());
    configIosVersion = config.readEntry(Config::IosVersion, QVariant(Config::DefaultIosVersion).toFloat());
    m_ui->unicodeComboBox->setCurrentText(config.readEntry(Config::UnicodeVersion, Config::DefaultUnicodeVersion));
    m_ui->iosComboBox->setCurrentText(config.readEntry(Config::IosVersion, Config::DefaultIosVersion));

    m_ui->maxFavouritesLabel->setHidden(true);
    categoriesChanged();
    filterActive = true;
    filterEmojiListView();
    validateEditingOptions();

    emit changed(true);
}


void EmojiRunnerConfig::save() {
    // Save general settings
    config.writeEntry(Config::GlobalSearch, m_ui->enableGlobalSearch->isChecked());
    config.writeEntry(Config::SingleRunnerModePaste, m_ui->singleRunnerModePaste->isChecked());
    config.writeEntry(Config::SearchByTags, m_ui->favouriteFilterTags_2->isChecked());
    config.writeEntry(Config::SearchByDescription, m_ui->favouriteFilterDescription_2->isChecked());
    config.writeEntry(Config::UnicodeVersion, m_ui->unicodeComboBox->currentText());
    config.writeEntry(Config::IosVersion, m_ui->iosComboBox->currentText());

    // Save disabled categories
    QString disabledCategories;
    const int categoryCount = m_ui->categoryListView->count();
    for (int i = 0; i < categoryCount; ++i) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) disabledCategories.append(item->text() + ";");
    }
    config.writeEntry(Config::DisabledCategories, disabledCategories);

    QList<Emoji> customEmojis;
    // Save favourites and search for custom emojis
    const int emojiCount = m_ui->emojiListView->count();
    QString favouriteIDs;
    for (int i = 0; i < emojiCount; ++i) {
        const auto *item = m_ui->emojiListView->item(i);
        const auto emoji = allEmojis.value(item->data(Qt::UserRole).toString());
        if (item->checkState() == Qt::Checked) {
            favouriteIDs.append(QString::number(emoji.id) + ";");
        }
        if (emoji.category == Config::CustomCategory) customEmojis.append(emoji);
    }
    config.writeEntry(Config::Favourites, favouriteIDs);

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

void EmojiRunnerConfig::filterEmojiListView() {
    if (!filterActive) return;

    const QString text = m_ui->favouriteFilter->text().toLower();
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
            const auto emoji = allEmojis.value(item->data(Qt::UserRole).toString());
            bool hidden = true;

            // If the category is disabled and it is not a favourite it should always be hidden
            if (item->checkState() != Qt::Checked && disabledEmojiCategoryNames.contains(emoji.category)) {
                item->setHidden(true);
                continue;
            }

            // If custom is selected only the custom entries are filtered by the criterias
            if (custom && emoji.category != Config::CustomCategory) {
                item->setHidden(true);
                continue;
            }

            if (filterName) {
                if (emoji.name.contains(text)) {
                    hidden = false;
                }
            }
            if (hidden && filterDescription) {
                if (emoji.description.contains(text)) hidden = false;
            }
            if (hidden && filterTags) {
                for (const auto &t:emoji.tags) if (t.contains(text)) hidden = false;
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

void EmojiRunnerConfig::unicodeVersionChanged() {
    configUnicodeVersion = m_ui->unicodeComboBox->currentText().toFloat();
    m_ui->favouriteFilter->clear();
    filterEmojiListView();
}

void EmojiRunnerConfig::iosVersionChanged() {
    configIosVersion = m_ui->iosComboBox->currentText().toFloat();
    m_ui->favouriteFilter->clear();
    filterEmojiListView();
}

void EmojiRunnerConfig::unhideAll() {
    const int itemCount = m_ui->emojiListView->count();
    const bool custom = favouriteFilters.contains("custom");

    for (int i = 0; i < itemCount; ++i) {
        auto *item = m_ui->emojiListView->item(i);
        const auto emoji = allEmojis.value(item->data(Qt::UserRole).toString());
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

void EmojiRunnerConfig::displayVisibleItems() const {
    int visibleItems = 0;
    const int count = this->m_ui->emojiListView->count();
    for (int i = 0; i < count; ++i) {
        const auto *item = this->m_ui->emojiListView->item(i);
        if (!item->isHidden()) ++visibleItems;
    }

    m_ui->favouriteVisibleElements->setText(QString::number(visibleItems) + " Elements");
}

void EmojiRunnerConfig::checkMaxFavourites() {
    int favourites = 0;
    const int count = this->m_ui->emojiListView->count();
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

void EmojiRunnerConfig::changeFontSize(int value) {
    auto f = QFont(m_ui->emojiListView->font());
    f.setPixelSize(value / 2);
    m_ui->emojiListView->setFont(f);
}

void EmojiRunnerConfig::addEmoji() {
    auto *popup = new EmojiRunnerPopup(this);
    popup->show();
    connect(popup, SIGNAL(finished(Emoji, QString)), this, SLOT(applyEmojiPopupResults(Emoji, QString)));
}

void EmojiRunnerConfig::editEmoji() {
    const auto *item = m_ui->emojiListView->currentItem();
    if (item != nullptr) {
        auto *popup = new EmojiRunnerPopup(this, allEmojis.value(item->data(Qt::UserRole).toString()));
        popup->show();
        connect(popup, SIGNAL(finished(Emoji, QString)), this, SLOT(applyEmojiPopupResults(Emoji, QString)));
    }
}

void EmojiRunnerConfig::deleteEmoji() {
    QMessageBox::StandardButton reply = QMessageBox::
    question(this, "Confirm Delete", "Do you want to delete this custom emoji ?",
             QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) delete m_ui->emojiListView->takeItem(m_ui->emojiListView->currentRow());
}

void EmojiRunnerConfig::applyEmojiPopupResults(const Emoji &emoji, const QString &originalName) {
    // Delete Dialog that emitted the signal
    delete sender();
    if (!allEmojis.contains(emoji.name) && originalName.isEmpty()) {
        m_ui->emojiListView->insertItem(0, Utilities::toListWidgetItem(emoji));
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
            m_ui->emojiListView->insertItem(row, Utilities::toListWidgetItem(emoji));
        } else {
            m_ui->emojiListView->insertItem(0, Utilities::toListWidgetItem(emoji));
            qInfo() << "Could not update" << emoji.name << originalName;
        }
    }
    allEmojis.insert(emoji.name, emoji);
}

void EmojiRunnerConfig::validateEditingOptions() {
    const auto *item = m_ui->emojiListView->currentItem();
    if (item == nullptr) {
        m_ui->editEmojiPushButton->setDisabled(true);
        m_ui->deleteEmojiPushButton->setDisabled(true);
        return;
    }
    const auto emoji = allEmojis.value(item->data(Qt::UserRole).toString());
    const bool disabled = emoji.category != "Custom";
    m_ui->editEmojiPushButton->setDisabled(disabled);
    m_ui->deleteEmojiPushButton->setDisabled(disabled);
}

void EmojiRunnerConfig::toggleFavouriteOptions() {
    const bool hide = !m_ui->favouriteFilterGroupBox->isHidden();
    m_ui->toggleFavouritesPushButton->setIcon(QIcon::fromTheme(hide ? "arrow-down" : "arrow-up"));
    m_ui->favouriteFilterGroupBox->setHidden(hide);
    m_ui->sortFavourites->setHidden(hide);
    m_ui->customButtonsWidget->setHidden(hide);
    m_ui->fontSizeContainerWidget->setHidden(hide);
}


#include "emojirunner_config.moc"

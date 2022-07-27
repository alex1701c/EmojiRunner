#include "emojirunner_config.h"
#include "emojirunner_popup.h"
#include <KSharedConfig>
#include <KPluginFactory>
#include <core/FileReader.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QDialog>
#include <QtWidgets/QInputDialog>
#include "core/utilities.h"
#include <core/Config.h>
#include "core/macros.h"

K_PLUGIN_FACTORY(EmojiRunnerConfigFactory, registerPlugin<EmojiRunnerConfig>();)

#define itemEmoji(item) item->data(Qt::UserRole).value<Emoji>()


EmojiRunnerConfigForm::EmojiRunnerConfigForm(QWidget *parent) : QWidget(parent) {
    setupUi(this);
}

EmojiRunnerConfig::EmojiRunnerConfig(QWidget *parent, const QVariantList &args) : KCModule(parent, args) {
    m_ui = new EmojiRunnerConfigForm(this);
    auto *layout = new QGridLayout(this);
    layout->addWidget(m_ui, 0, 0);

    createConfigFile();
    config = KSharedConfig::openConfig(Config::ConfigFilePath)->group(Config::RootGroup);
    config.config()->reparseConfiguration();

    FileReader reader(config);
    emojiCategories = reader.getEmojiCategories(true);
    disabledEmojiCategoryNames = reader.disabledCategories;

    std::sort(emojiCategories.begin(), emojiCategories.end(), [](const EmojiCategory &c1, const EmojiCategory &c2) -> bool {
        if (c1.name == Config::Config::CustomCategory) return true;
        if (c1.name == Config::SmileysEmotionsCategory && c2.name != Config::CustomCategory) return true;
        return c1.name < c2.name;
    });

}

EmojiRunnerConfig::~EmojiRunnerConfig() {
    for (auto &category: emojiCategories) {
        if (category.name == Config::FavouritesCategory) {
            continue;
        }
        category.emojis.clear();
    }
}

void EmojiRunnerConfig::load() {
    // Load method gets also called when exiting config dialog without saving
    static bool loaded = false;
    if (loaded) {
        return;
    }
    loaded = true;

    m_ui->enableGlobalSearch->setChecked(config.readEntry(Config::GlobalSearch, true));
    m_ui->favouriteFilterDescription_2->setChecked(config.readEntry(Config::SearchByDescription, false));
    m_ui->favouriteFilterTags_2->setChecked(config.readEntry(Config::SearchByTags, false));

    // Load categories
    for (const auto &category: qAsConst(emojiCategories)) {
        if (category.name == Config::FavouritesCategory) continue;
        auto *item = new QListWidgetItem();
        item->setText(category.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (category.name == Config::CustomCategory) customEntriesExist = true;
        item->setCheckState(disabledEmojiCategoryNames.contains(category.name) ? Qt::Unchecked : Qt::Checked);
        m_ui->categoryListView->addItem(item);
    }

    // Load favourites at top, apply sort from config
    QList<Emoji> favouriteEmojisToAdd;
    for (const auto &category: qAsConst(emojiCategories)) {
        if (category.name != Config::FavouritesCategory) continue;
        for (const auto &emoji: category.emojis) {
            favouriteEmojisToAdd.append(emoji);
        }
    }
    std::sort(favouriteEmojisToAdd.begin(), favouriteEmojisToAdd.end(), [](const Emoji &e1, const Emoji &e2) -> bool {
        return e1.favourite > e2.favourite;
    });
    for (const auto &emoji: qAsConst(favouriteEmojisToAdd)) {
        m_ui->emojiListView->addItem(Utilities::toListWidgetItem(emoji));
    }

    // Load other emojis
    for (const auto &category: qAsConst(emojiCategories)) {
        if (category.name == Config::FavouritesCategory) continue;
        for (const auto &emoji:category.emojis) {
            if (emoji.favourite != 0) continue;
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
    validateEditingOptions();
    categoriesChanged();
    filtersChanged();
    connectSignals();

    Q_EMIT changed(true);
}


void EmojiRunnerConfig::save() {
    // Save general settings
    config.writeEntry(Config::GlobalSearch, m_ui->enableGlobalSearch->isChecked());
    config.writeEntry(Config::SearchByTags, m_ui->favouriteFilterTags_2->isChecked());
    config.writeEntry(Config::SearchByDescription, m_ui->favouriteFilterDescription_2->isChecked());
    config.writeEntry(Config::UnicodeVersion, m_ui->unicodeComboBox->currentText());
    config.writeEntry(Config::IosVersion, m_ui->iosComboBox->currentText());

    // Save disabled categories
    QString disabledCategories;
    const int categoryCount = m_ui->categoryListView->count();
    for (int i = 0; i < categoryCount; ++i) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) disabledCategories.append(item->text() + ';');
    }
    config.writeEntry(Config::DisabledCategories, disabledCategories);

    QList<Emoji> customEmojis;
    // Save favourites and search for custom emojis
    const int emojiCount = m_ui->emojiListView->count();
    QString favouriteIDs;
    for (int i = 0; i < emojiCount; ++i) {
        const auto *item = m_ui->emojiListView->item(i);
        const auto &emoji = itemEmoji(item);
        if (item->checkState() == Qt::Checked) {
            favouriteIDs.append(QString::number(emoji.id) + ';');
        }
        if (emoji.category == Config::CustomCategory) customEmojis.append(emoji);
    }
    config.writeEntry(Config::Favourites, favouriteIDs);

    if (!customEmojis.isEmpty()) Emoji::writeToJSONFile(customEmojis);

    config.sync();
    config.config()->sync();
}

void EmojiRunnerConfig::defaults() {
    m_ui->enableGlobalSearch->setChecked(true);
    m_ui->favouriteFilterDescription_2->setChecked(false);
    m_ui->favouriteFilterTags_2->setChecked(false);
    m_ui->unicodeComboBox->setCurrentText(QSL("11"));
    m_ui->iosComboBox->setCurrentText(QSL("13"));

    for (int i = 0; i < m_ui->categoryListView->count(); ++i) {
        m_ui->categoryListView->item(i)->setCheckState(Qt::Checked);
    }

    m_ui->favouriteFilter->clear();
    m_ui->favouriteFilterName->setChecked(true);
    m_ui->favouriteFilterTags->setChecked(false);
    m_ui->favouriteFilterDescription->setChecked(false);
    m_ui->sortFavourites->setChecked(false);

    Q_EMIT changed(true);
}


void EmojiRunnerConfig::connectSignals() {
    // Initialize function pointers that require method overloading
    const auto comboBoxIndexChanged = static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged);
    const auto listWidgetRowChanged = static_cast<void (QListWidget::*)(int)>(&QListWidget::currentRowChanged);
    const auto listWidgetItemChanged = static_cast<void (QListWidget::*)(QListWidgetItem *)>(&QListWidget::itemChanged);

    // Connect slots for filters
    connect(m_ui->favouriteFilter, &QLineEdit::textChanged, this, &EmojiRunnerConfig::filterEmojiListView);
    connect(m_ui->favouriteFilterName, &QCheckBox::clicked, this, &EmojiRunnerConfig::filtersChanged);
    connect(m_ui->favouriteFilterDescription, &QCheckBox::clicked, this, &EmojiRunnerConfig::filtersChanged);
    connect(m_ui->favouriteFilterTags, &QCheckBox::clicked, this, &EmojiRunnerConfig::filtersChanged);
    connect(m_ui->favouriteFilterCustom, &QCheckBox::clicked, this, &EmojiRunnerConfig::filtersChanged);
    // Unicode Versions change
    connect(m_ui->unicodeComboBox, comboBoxIndexChanged, this, &EmojiRunnerConfig::unicodeVersionChanged);
    connect(m_ui->iosComboBox, comboBoxIndexChanged, this, &EmojiRunnerConfig::iosVersionChanged);
    // Disable categories
    connect(m_ui->categoryListView, listWidgetItemChanged, this, &EmojiRunnerConfig::categoriesChanged);
    // Sort favourites
    connect(m_ui->sortFavourites, &QCheckBox::clicked, this, &EmojiRunnerConfig::showOnlyFavourites);
    connect(m_ui->emojiListView, listWidgetItemChanged, this, &EmojiRunnerConfig::checkMaxFavourites);
    // For Drag/Drop events
    // Slider for font size
    connect(m_ui->fontSizeSlider, &QSlider::valueChanged, this, &EmojiRunnerConfig::changeFontSize);
    // Buttons for adding/updating/deleting emojis
    connect(m_ui->emojiListView, listWidgetRowChanged, this, &EmojiRunnerConfig::validateEditingOptions);
    connect(m_ui->addEmojiPushButton, &QCheckBox::clicked, this, &EmojiRunnerConfig::addEmoji);
    connect(m_ui->editEmojiPushButton, &QCheckBox::clicked, this, &EmojiRunnerConfig::editEmoji);
    connect(m_ui->deleteEmojiPushButton, &QCheckBox::clicked, this, &EmojiRunnerConfig::deleteEmoji);
    // Toggle favourite search options
    connect(m_ui->toggleFavouritesPushButton, &QCheckBox::clicked, this, &EmojiRunnerConfig::toggleFavouriteOptions);
}

void EmojiRunnerConfig::filterEmojiListView() {
    const QString text = m_ui->favouriteFilter->text().toLower();
    const bool unhideAllEnabledEmojis = filterName && filterTags && filterTags && text.isEmpty();

    for (int i = 0; i < m_ui->emojiListView->count(); ++i) {
        // Process events, otherwise the search delays the GUI updates
        if (i % 100 == 0) {
            QApplication::processEvents();
        }

        auto *item = m_ui->emojiListView->item(i);
        const Emoji &emoji = itemEmoji(item);

        // If unhideAllEnabledEmojis is true the other tests do not matter
        if (unhideAllEnabledEmojis) {
            item->setHidden(
                    ((
                             !emoji.matchesVersions(configUnicodeVersion, configIosVersion) ||
                             disabledEmojiCategoryNames.contains(emoji.category)
                     )
                     && item->checkState() == Qt::Unchecked) ||
                    (filterCustom && emoji.category != Config::CustomCategory)
            );
        } else {
            bool hidden = true;
            // If the category is disabled and it is not a favourite it should always be hidden
            if (item->checkState() != Qt::Checked && disabledEmojiCategoryNames.contains(emoji.category)) {
                item->setHidden(true);
                continue;
            }

            // If custom is selected only the custom entries are filtered by the criterias
            if (filterCustom && emoji.category != Config::CustomCategory) {
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
                for (const auto &t:emoji.tags)
                    if (t.contains(text)) {
                        hidden = false;
                    }
            }
            if (emoji.matchesVersions(configUnicodeVersion, configIosVersion) || item->checkState() == Qt::Checked) {
                item->setHidden(hidden);
            } else {
                item->setHidden(true);
            }
        }
    }
    displayVisibleItems();
}


void EmojiRunnerConfig::filtersChanged() {
    // Enable/Disable the filter checkboxes and trigger search event
    filterName = m_ui->favouriteFilterName->isChecked();
    filterTags = m_ui->favouriteFilterTags->isChecked();
    filterDescription = m_ui->favouriteFilterDescription->isChecked();

    // The custom filter changes the results, even if there is no query typed
    bool customFilterChanged = false;
    filterCustom = m_ui->favouriteFilterCustom->isChecked();
    if (filterCustom) {
        if (!customFilterWasChecked) customFilterChanged = true;
        customFilterWasChecked = true;
    } else {
        if (customFilterWasChecked) {
            customFilterChanged = true;
        }
        customFilterWasChecked = false;
    }

    if (!m_ui->favouriteFilter->text().isEmpty() || customFilterChanged) {
        filterEmojiListView();
    }
}

void EmojiRunnerConfig::categoriesChanged() {
    const QStringList previouslyDisabled = disabledEmojiCategoryNames;
    disabledEmojiCategoryNames.clear();

    // Update list of disabled categories
    for (int i = 0; i < m_ui->categoryListView->count(); ++i) {
        const auto *item = m_ui->categoryListView->item(i);
        if (item->checkState() == Qt::Unchecked) {
            disabledEmojiCategoryNames.append(item->text());
        }
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
        for (int i = 0; i < itemCount; ++i) {
            auto *item = m_ui->emojiListView->item(i);
            item->setHidden(item->checkState() == Qt::Unchecked);
        }
    } else {
        filterEmojiListView();
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

void EmojiRunnerConfig::displayVisibleItems() {
    int visibleItems = 0;
    for (int i = 0; i < this->m_ui->emojiListView->count(); ++i) {
        if (!this->m_ui->emojiListView->item(i)->isHidden()) {
            ++visibleItems;
        }
    }

    m_ui->favouriteVisibleElements->setText(QString::number(visibleItems) + " Elements");
}

void EmojiRunnerConfig::checkMaxFavourites() {
    int favourites = 0;
    for (int i = 0; i < this->m_ui->emojiListView->count(); ++i) {
        if (this->m_ui->emojiListView->item(i)->checkState() == Qt::Checked) {
            ++favourites;
        }
    }

    m_ui->maxFavouritesLabel->setHidden(favourites <= 20);
}

void EmojiRunnerConfig::changeFontSize(int value) {
    auto f = QFont(m_ui->emojiListView->font());
    f.setPixelSize(value / 2);
    m_ui->emojiListView->setFont(f);
}

void EmojiRunnerConfig::addEmoji() {
    auto *popup = new EmojiRunnerPopup(this);
    popup->show();
    connect(popup, &EmojiRunnerPopup::finished, this, &EmojiRunnerConfig::applyEmojiPopupResults);
}

void EmojiRunnerConfig::editEmoji() {
    const auto *item = m_ui->emojiListView->currentItem();
    if (item != nullptr) {
        auto *popup = new EmojiRunnerPopup(this, itemEmoji(item), m_ui->emojiListView->currentRow());
        popup->show();
        connect(popup, &EmojiRunnerPopup::finished, this, &EmojiRunnerConfig::applyEmojiPopupResults);
    }
}

void EmojiRunnerConfig::deleteEmoji() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, QSL("Confirm Delete"),
            QSL("Do you want to delete this custom emoji ?"), QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        delete m_ui->emojiListView->takeItem(m_ui->emojiListView->currentRow());
    }
}

void EmojiRunnerConfig::applyEmojiPopupResults(const Emoji &emoji, const int idx) {
    // Delete Dialog that emitted the signal
    delete sender();
    if (idx == -1) {
        m_ui->emojiListView->insertItem(0, Utilities::toListWidgetItem(emoji));
        // Add "Custom" category to category list view
        if (!customEntriesExist) {
            customEntriesExist = true;
            auto *item = new QListWidgetItem();
            item->setText(QSL("Custom"));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            m_ui->categoryListView->addItem(item);
        }
    } else {
        auto *item = m_ui->emojiListView->item(idx);
        const int row = m_ui->emojiListView->row(item);
        delete m_ui->emojiListView->takeItem(row);
        m_ui->emojiListView->insertItem(row, Utilities::toListWidgetItem(emoji));
    }
}

void EmojiRunnerConfig::validateEditingOptions() {
    const auto *item = m_ui->emojiListView->currentItem();
    if (item == nullptr) {
        m_ui->editEmojiPushButton->setDisabled(true);
        m_ui->deleteEmojiPushButton->setDisabled(true);
    } else {
        const auto &emoji = itemEmoji(item);
        const bool disabled = emoji.category != Config::CustomCategory;
        m_ui->editEmojiPushButton->setDisabled(disabled);
        m_ui->deleteEmojiPushButton->setDisabled(disabled);
    }
}

void EmojiRunnerConfig::toggleFavouriteOptions() {
    const bool hide = !m_ui->favouriteFilterGroupBox->isHidden();
    m_ui->toggleFavouritesPushButton->setIcon(QIcon::fromTheme(hide ? QSL("arrow-down") : QSL("arrow-up")));
    m_ui->favouriteFilterGroupBox->setHidden(hide);
    m_ui->sortFavourites->setHidden(hide);
    m_ui->customButtonsWidget->setHidden(hide);
    m_ui->fontSizeContainerWidget->setHidden(hide);
}


#include "emojirunner_config.moc"

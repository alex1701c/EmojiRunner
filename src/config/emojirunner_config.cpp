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

// TODO Filter disabled categories
// TODO Implement Mode Favourites Functionality
// TODO Handle Save
// TODO Handle Defaults
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

    // TODO Show count
    // TODO Connect signals when unicode changes
    // TODO Filter unicode version

    // Connect slots for filters
    connect(m_ui->favouriteFilter, SIGNAL(textChanged(QString)), this, SLOT(filterFavourites()));
    connect(m_ui->favouriteFilterVersions, SIGNAL(clicked(bool)), this, SLOT(filterFavourites()));
    connect(m_ui->favouriteFilterName, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterDescription, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterTags, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    // Unicode Versions change => eventually reload filters
    connect(m_ui->unicodeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unicodeVersionsChanged()));
    // Disable categories
    connect(m_ui->applyCategoryChanges, SIGNAL(clicked(bool)), this, SLOT(categoriesApplyChanges()));

    filtersChanged(false);

}

void EmojiRunnerConfig::load() {

    m_ui->enableGlobalSearch->setChecked(config.readEntry("globalSearch", "true") == "true");

    // Load categories
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") continue;
        auto *item = new QListWidgetItem();
        item->setText(category.name);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        disabledEmojis.contains(category.name) ? item->setCheckState(Qt::Unchecked) : item->setCheckState(Qt::Checked);;
        m_ui->categoryListView->addItem(item);
    }

    // Load Unicode versions
    for (const auto &unicodeVersion:unicodeVersions) {
        m_ui->unicodeComboBox->addItem(QString::number(unicodeVersion));
    }
    if (config.readEntry("unicodeVersion").isEmpty()) m_ui->unicodeComboBox->setCurrentIndex(m_ui->unicodeComboBox->count() - 1);
    else m_ui->unicodeComboBox->setCurrentText(config.readEntry("unicodeVersion"));
    // Load Ios fallback versions
    for (const auto &iosVersion:iosVersions) {
        m_ui->iosComboBox->addItem(QString::number(iosVersion));
    }
    if (config.readEntry("unicodeVersion").isEmpty()) m_ui->iosComboBox->setCurrentIndex(m_ui->iosComboBox->count() - 1);
    else m_ui->iosComboBox->setCurrentText(config.readEntry("iosVersion"));

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

    m_ui->favouriteVisibleElements->setText(QString::number(m_ui->favouriteListView->count()) + " Elements");

    emit changed(false);
}


void EmojiRunnerConfig::save() {
    emit changed();
}

void EmojiRunnerConfig::defaults() {
    emit changed(true);
}

void EmojiRunnerConfig::filterFavourites() {
    const float unicodeVersion = m_ui->unicodeComboBox->itemText(m_ui->unicodeComboBox->currentIndex()).toFloat();
    const float iosVersion = m_ui->iosComboBox->itemText(m_ui->iosComboBox->currentIndex()).toFloat();
    bool unicode = m_ui->favouriteFilterVersions->isChecked();
    if (unicodeVersion == 12 && iosVersion == 13) unicode = false;
    const QString text = m_ui->favouriteFilter->text();
    int visibleItems = 0;
    int count = m_ui->favouriteListView->count();

    if (text.isEmpty()) {
        // Show all entries
        if (!unicode) {
            for (int i = 0; i < count; i++) {
                // TODO Very slow!
                m_ui->favouriteListView->item(i)->setHidden(false);
            }
            visibleItems = m_ui->favouriteListView->count();
        } else {
            //
            for (int i = 0; i < count; i++) {
                const auto emoji = allEmojis.value(m_ui->favouriteListView->item(i)->data(1).toString());
                if (emoji.unicodeVersion != 0 && emoji.unicodeVersion <= unicodeVersion) {
                    m_ui->favouriteListView->item(i)->setHidden(false);
                    ++visibleItems;
                } else if (emoji.unicodeVersion == 0 && emoji.iosVersion <= iosVersion) {
                    m_ui->favouriteListView->item(i)->setHidden(false);
                    ++visibleItems;
                } else {
                    m_ui->favouriteListView->item(i)->setHidden(true);
                }
            }
        }
    } else {
        for (int i = 0; i < count; ++i) {
            auto *item = m_ui->favouriteListView->item(i);
            bool hidden = true;
            const auto emoji = allEmojis.value(item->data(1).toString());

            // Skip if unicode/ios version is to great
            if (unicode && (emoji.unicodeVersion > unicodeVersion || (emoji.unicodeVersion == 0 && emoji.iosVersion > iosVersion))) {
                item->setHidden(true);
                continue;
            }

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
            if (!hidden) ++visibleItems;
            item->setHidden(hidden);
        }
    }

    m_ui->favouriteVisibleElements->setText(QString::number(visibleItems) + " Elements");
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

    if (reloadFilter)filterFavourites();
}

void EmojiRunnerConfig::unicodeVersionsChanged() {
    if (m_ui->favouriteFilterVersions->isChecked()) filterFavourites();
}

void EmojiRunnerConfig::categoriesApplyChanges() {
    // TODO Remove all emoji from disabled categories that are no favourites/have not been selected
    const QStringList previouslyDisabled = disabledEmojis;
    QStringList newFavourites;
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
            if (item->checkState() == Qt::Checked || emoji.favourite != 0) {
                newFavourites.append(item->text());
                continue;
            }
            remove.insert(0, i);
        }
    }
    for (int rm:remove)m_ui->favouriteListView->model()->removeRow(rm);

    // Get newly enabled items
    QStringList newlyEnabled;
    for (const auto &c:previouslyDisabled) {
        if (!disabledEmojis.contains(c)) newlyEnabled.append(c);
    }
    qInfo() << newFavourites;
    // Add newly installed emojis
    for (const auto &c:emojiCategories) {
        if (!newlyEnabled.contains(c.name)) continue;
        for (const auto &emoji:c.emojis.values()) {
            const QString text = emoji.emoji + " " + QString(emoji.name).replace('_', ' ');
            if (newFavourites.contains(text)) {
                // TODO Doublicates get inserted
                qInfo() << "SKIP" << emoji.name;
                continue;
            }
            auto *item = new QListWidgetItem(text);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            item->setData(1, emoji.name);
            m_ui->favouriteListView->addItem(item);
        }
    }

    m_ui->favouriteVisibleElements->setText(QString::number(m_ui->favouriteListView->count()) + " Elements");
}


#include "emojirunner_config.moc"

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

    emojiCategories = FileReader::readJSONFile();
    config = KSharedConfig::openConfig("krunnerrc")->group("Runners").group("EmojiRunner");
    disabledEmojis = config.readEntry("disabledCategories", "").split(";", QString::SplitBehavior::SkipEmptyParts);

    std::sort(emojiCategories.begin(), emojiCategories.end(), [](EmojiCategory c1, EmojiCategory c2) -> bool {
        if (c1.name == "Smileys & Emotion") return true;
        return c1.name < c2.name;
    });

    connect(m_ui->favouriteFilter, SIGNAL(textChanged(QString)), this, SLOT(filterFavourites()));
    connect(m_ui->favouriteFilterName, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterDescription, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));
    connect(m_ui->favouriteFilterTags, SIGNAL(clicked(bool)), this, SLOT(filtersChanged()));

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

    // Load Unicode/Ios versions
    m_ui->unicodeComboBox->addItems(unicodeVersions);
    if (config.readEntry("unicodeVersion").isEmpty()) m_ui->unicodeComboBox->setCurrentIndex(m_ui->unicodeComboBox->count() - 1);
    else m_ui->unicodeComboBox->setCurrentText(config.readEntry("unicodeVersion"));
    m_ui->iosComboBox->addItems(iosVersions);
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

    emit changed(false);
}


void EmojiRunnerConfig::save() {
    emit changed();
}

void EmojiRunnerConfig::defaults() {
    emit changed(true);
}

void EmojiRunnerConfig::filterFavourites() {
    const QString text = m_ui->favouriteFilter->text();
    int count = m_ui->favouriteListView->count();

    if (text.isEmpty()) {
        for (int i = 0; i < count; i++) {
            m_ui->favouriteListView->item(i)->setHidden(false);
        }
    } else {
        for (int i = 0; i < count; i++) {
            auto *item = m_ui->favouriteListView->item(i);
            bool hidden = true;
            const auto emoji = allEmojis.value(item->data(1).toString());

            // Search properties based on selected filters
            // glasses tags and description
            if (favouriteFilters.contains("name")) {
                if (emoji.name.contains(text, Qt::CaseInsensitive) ||
                    emoji.name.contains(QString(text).replace(' ', '_'), Qt::CaseInsensitive)) {
                    hidden = false;
                }
            }
            if (hidden && favouriteFilters.contains("description")) {
                if (emoji.description.contains(text, Qt::CaseInsensitive)) hidden = false;
            }
            if (hidden && favouriteFilters.contains("tags")) {
                for (const auto &t:emoji.tags) if (t.contains(text, Qt::CaseInsensitive)) hidden = false;
            }

            item->setHidden(hidden);
        }
    }
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


#include "emojirunner_config.moc"

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

}

void EmojiRunnerConfig::load() {

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
            auto *item = new QListWidgetItem(emoji.emoji + " " + emoji.name);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);
            m_ui->favouriteListView->addItem(item);
        }
    }
    // Load other emojis
    for (const auto &category:emojiCategories) {
        if (category.name == "Favourites") continue;
        for (const auto &emoji:category.emojis.values()) {
            if (favouriteNames.contains(emoji.name)) continue;
            auto *item = new QListWidgetItem(emoji.emoji + " " + emoji.name);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
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


#include "emojirunner_config.moc"

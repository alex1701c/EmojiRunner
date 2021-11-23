#include "emojirunner_popup.h"

#include <core/Config.h>

EmojiRunnerPopup::EmojiRunnerPopup(QWidget *parent, Emoji *emoji, int idx) : QDialog(parent) {
    setupUi(this);

    this->idx = idx;
    this->emoji = emoji;
    setDataOfEmoji();

    connect(this->buttonBox, &QDialogButtonBox::accepted, this, &EmojiRunnerPopup::writeDataToEmoji);
    connect(this->nameLineEdit, &QLineEdit::textChanged, this, &EmojiRunnerPopup::validateButtonBox);
    connect(this->emojiLineEdit, &QLineEdit::textChanged, this, &EmojiRunnerPopup::validateButtonBox);
    validateButtonBox();
}

void EmojiRunnerPopup::setDataOfEmoji() {
    this->emojiLineEdit->setText(emoji->emoji);
    this->nameLineEdit->setText(emoji->name);
    this->tagsLineEdit->setText(emoji->tags.join(','));
    this->descriptionLineEdit->setText(emoji->description);
}

void EmojiRunnerPopup::writeDataToEmoji() {
    emoji->category = Config::CustomCategory;
    emoji->emoji = this->emojiLineEdit->text();
    emoji->name = this->nameLineEdit->text();
    emoji->tags = this->tagsLineEdit->text().toLower().split(',', Qt::SkipEmptyParts);
    emoji->description = this->descriptionLineEdit->text();
    // Give name as parameter to find the existing item if the emoji has been updated
    emit finished(this->emoji, idx);
}

void EmojiRunnerPopup::validateButtonBox() {
    this->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(
            this->nameLineEdit->text().isEmpty() || this->emojiLineEdit->text().isEmpty());
}

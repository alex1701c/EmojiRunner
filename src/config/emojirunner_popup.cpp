#include "emojirunner_popup.h"

#include <utility>

EmojiRunnerPopup::EmojiRunnerPopup(QWidget *parent, Emoji emoji) : QDialog(parent) {
    setupUi(this);

    this->emoji = std::move(emoji);
    this->originalName = this->emoji.name;
    setDataOfEmoji();

    connect(this->buttonBox, SIGNAL(accepted()), this, SLOT(writeDataToEmoji()));
    connect(this->nameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateButtonBox()));
    connect(this->emojiLineEdit, SIGNAL(textChanged(QString)), this, SLOT(validateButtonBox()));
    validateButtonBox();
}

void EmojiRunnerPopup::setDataOfEmoji() {
    this->emojiLineEdit->setText(emoji.emoji);
    this->nameLineEdit->setText(emoji.name);
    this->tagsLineEdit->setText(emoji.tags.join(","));
    this->descriptionLineEdit->setText(emoji.description);
}

void EmojiRunnerPopup::writeDataToEmoji() {
    emoji.category = "Custom";
    emoji.emoji = this->emojiLineEdit->text();
    emoji.name = this->nameLineEdit->text();
    emoji.tags = this->tagsLineEdit->text().toLower().split(",", QString::SkipEmptyParts);
    emoji.description = this->descriptionLineEdit->text();
    // Give name as parameter to find the existing item if the emoji has been updated
    emit finished(this->emoji, this->originalName);
}

void EmojiRunnerPopup::validateButtonBox() {
    this->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(
            this->nameLineEdit->text().isEmpty() || this->emojiLineEdit->text().isEmpty());
}

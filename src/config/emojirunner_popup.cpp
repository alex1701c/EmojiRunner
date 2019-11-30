//
// Created by alex on 30.11.19.
//

#include "emojirunner_popup.h"

#include <utility>

EmojiRunnerPopup::EmojiRunnerPopup(QWidget *parent, Emoji emoji) : QDialog(parent) {
    setupUi(this);

    this->emoji = std::move(emoji);
    this->originalName = this->emoji.displayName;
    setDataOfEmoji();

    connect(this->buttonBox, SIGNAL(accepted()), this, SLOT(writeDataToEmoji()));
}

void EmojiRunnerPopup::setDataOfEmoji() {
    this->categoryComboBox->setCurrentText(emoji.category);
    this->emojiLineEdit->setText(emoji.emoji);
    this->nameLineEdit->setText(emoji.displayName);
    this->tagsLineEdit->setText(emoji.tags.join(","));
    this->descriptionLineEdit->setText(emoji.description);
}

void EmojiRunnerPopup::writeDataToEmoji() {
    emoji.category = this->categoryComboBox->currentText();
    emoji.emoji = this->emojiLineEdit->text();
    emoji.name = this->nameLineEdit->text();
    emoji.displayName = QString(emoji.name).replace("_", " ");
    emoji.tags = this->tagsLineEdit->text().split(",", QString::SkipEmptyParts);
    emoji.description = this->descriptionLineEdit->text();
    // Give name as parameter to find the existing item if the emoji has been updated
    emit finished(this->emoji, this->originalName);
}

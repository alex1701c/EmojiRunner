#ifndef EMOJIRUNNER_EMOJIRUNNER_POPUP_H
#define EMOJIRUNNER_EMOJIRUNNER_POPUP_H

#include <QtCore>
#include <QtWidgets>
#include <core/Emoji.h>
#include "ui_emojirunner_popup.h"

class EmojiRunnerPopup : public QDialog, public Ui::Dialog {
Q_OBJECT
    QString originalName;
    Emoji emoji;
public:
    explicit EmojiRunnerPopup(QWidget *parent, Emoji emoji = Emoji());

    void setDataOfEmoji();

public Q_SLOTS:

    void writeDataToEmoji();

    void validateButtonBox();

Q_SIGNALS:

    void finished(Emoji, QString);
};


#endif //EMOJIRUNNER_EMOJIRUNNER_POPUP_H

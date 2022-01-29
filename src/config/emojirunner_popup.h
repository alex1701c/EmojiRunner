#ifndef EMOJIRUNNER_EMOJIRUNNER_POPUP_H
#define EMOJIRUNNER_EMOJIRUNNER_POPUP_H

#include <QtCore>
#include <QtWidgets>
#include <core/Emoji.h>
#include "ui_emojirunner_popup.h"

class EmojiRunnerPopup : public QDialog, public Ui::Dialog {
Q_OBJECT
    Emoji emoji;
    // Index of item that gets changes/created
    int idx;
public:
    explicit EmojiRunnerPopup(QWidget *parent, Emoji emoji = {}, int idx = -1);

    void setDataOfEmoji();

public Q_SLOTS:

    void writeDataToEmoji();

    void validateButtonBox();

Q_SIGNALS:

    void finished(Emoji &, int);
};


#endif //EMOJIRUNNER_EMOJIRUNNER_POPUP_H

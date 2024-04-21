#ifndef EMOJIRUNNER_EMOJIRUNNER_POPUP_H
#define EMOJIRUNNER_EMOJIRUNNER_POPUP_H

#include "ui_emojirunner_popup.h"
#include <core/Emoji.h>

#include <QDialog>

class EmojiRunnerPopup : public QDialog, public Ui::Dialog
{
    Q_OBJECT

public:
    explicit EmojiRunnerPopup(QWidget *parent, Emoji emoji = {}, int idx = -1);

    void setDataOfEmoji();
    void writeDataToEmoji();
    void validateButtonBox();

Q_SIGNALS:
    void finished(Emoji &, int);

private:
    Emoji emoji;
    // Index of item that gets changes/created
    int idx;
};

#endif // EMOJIRUNNER_EMOJIRUNNER_POPUP_H

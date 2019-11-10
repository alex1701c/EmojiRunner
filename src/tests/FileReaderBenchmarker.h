//
// Created by alex on 10.11.19.
//


#ifndef EMOJIRUNNER_FILEREADERBENCHMARKER_H
#define EMOJIRUNNER_FILEREADERBENCHMARKER_H

#include <QTest>
#include <iostream>
#include <KSharedConfig>
#include <KConfigGroup>
#include "../FileReader.h"

class FileReaderBenchmarker : public QObject {
Q_OBJECT


private slots:

    /*
    // 9,196 msecs per iteration
    static void testReadEnabledOld() {
        QBENCHMARK {
            for (int i = 0; i < 1000; ++i) {
                FileReader::readJSONFile(false);
            }
        }
    }*/
    //  7,574 msecs per iteration
    static void testReadEnabledNew() {
        QBENCHMARK {
            for (int i = 0; i < 1000; ++i) {
                FileReader::getEmojiCategories(false);
            }
        }
    }

    // 9,374 msecs per iteration
    static void testReadAllNew() {
        QBENCHMARK {
            for (int i = 0; i < 1000; ++i) {
                FileReader::getEmojiCategories(true);
            }
        }
    }

    /*
    // 10,809 msecs per iteration
    static void testReadAllOld() {
        QBENCHMARK {
            for (int i = 0; i < 1000; ++i) {
                FileReader::readJSONFile(true);
            }
        }
    }*/
};

QTEST_MAIN(FileReaderBenchmarker)

#endif //EMOJIRUNNER_FILEREADERBENCHMARKER_H

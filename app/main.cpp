/*******************************************************************************
 * Copyright [2018] <青岛艾普智能仪器有限公司>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       串口调试助手
*******************************************************************************/
#include <QApplication>
#include <QTranslator>
#include <QFile>
#include <QDir>

#include "appserial.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QFile file;
    file.setFileName("./qrc/qt_zh_CN.qm");
    if (file.exists()) {
        QTranslator qtTran;
        if (qtTran.load("./qrc/qt_zh_CN.qm")) {
            a.installTranslator(&qtTran);
        }
    }
    file.setFileName("./qrc/qtbase_zh_CN.qm");
    if (file.exists()) {
        QTranslator qtBase;
        if (qtBase.load("./qrc/qtbase_zh_CN.qm"))
            a.installTranslator(&qtBase);
    }
    file.setFileName("./qss/darkBlack.qss");
    if (file.exists()) {
        if (file.open(QFile::ReadOnly)) {
            qApp->setStyleSheet(QLatin1String(file.readAll()));
        }
    }
    AppSerial w;
    w.show();

    return a.exec();
}

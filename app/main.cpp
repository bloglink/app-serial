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

#include "appserial.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTran;
    qtTran.load(":/qt_zh_CN.qm");
    a.installTranslator(&qtTran);

    QTranslator qtBase;
    qtBase.load(":/qtbase_zh_CN.qm");
    a.installTranslator(&qtBase);

    QFile file;
    QString qss;
    file.setFileName(":/qss_black.css");
    file.open(QFile::ReadOnly);
    qss = QLatin1String(file.readAll());
    qApp->setStyleSheet(qss);

    AppSerial w;
    w.show();

    return a.exec();
}

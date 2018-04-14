/*******************************************************************************
 * Copyright [2018] <青岛艾普智能仪器有限公司>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       串口调试助手
*******************************************************************************/
#ifndef APPSERIAL_H
#define APPSERIAL_H

#include <QMainWindow>
#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QLayout>
#include <QListView>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSettings>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QElapsedTimer>
#include <QTableWidgetItem>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QHideEvent>
#include <QApplication>
#include <QDateTime>

#ifndef __linux__
#include <qt_windows.h>
#endif

class AppSerial : public QMainWindow
{
    Q_OBJECT

public:
    explicit AppSerial(QWidget *parent = 0);
    ~AppSerial();
private slots:
    void initUI();
    void initSkin();
    void initTitle();
    void initLayout();
    void initRecvBar();
    void initSendBar();
    void initSettings();
    void saveSettings();
    void openSerialDev();
    void recvSerialDev();
    void sendSerialDev();
    void sendSerialCurr();
    void sendSerialList();
    void stopSerialLoop();
    void sendSerialMsg(QByteArray msg);
    void display(QByteArray msg);
    void appendCurrentItem();
    void deleteCurrentItem();
    void updateCurrentItem(int r, int c);
    virtual void hideEvent(QHideEvent *e);
private:
    QGridLayout *layout;
    QTextEdit *EditGetText;
    QComboBox *boxDevPorts;
    QComboBox *boxBaudRate;
    QComboBox *boxNoParity;
    QComboBox *boxDataBits;
    QComboBox *boxStopBits;
    QCheckBox *boxAutoSpace;
    QCheckBox *boxGetToHEX;
    QLineEdit *EditPutTime;
    QLineEdit *EditPutRate;
    QCheckBox *boxLoopSend;
    QCheckBox *boxPutToHEX;
    QLineEdit *EditPutText;
    QTableWidget *tabList;

    QStringList tabTexts;
    QStringList tabNotes;

    QSerialPort *com;
    QTimer *timer;
    int timeOut;
    int sendOut;
};

#endif // APPSERIAL_H

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

#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QLayout>
#include <QDateTime>
#include <QListView>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSettings>
#include <QHideEvent>
#include <QPushButton>
#include <QMainWindow>
#include <QApplication>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#ifndef __linux__
#include <qt_windows.h>
#endif

class AppSerial : public QMainWindow
{
    Q_OBJECT

public:
    explicit AppSerial(QWidget *parent = 0);
private slots:
    void initUI();
    void initTitle();
    void initLayout();
    void initRecvText();
    void initRecvPort();
    void initRecvItem();
    void initSendView();
    void initSettings();
    void saveSettings();
    void openSerialDev();
    void sendSerialDat();
    void sendSerialDev();
    void recvSerialDev();
    void sendSerialMsg(QByteArray msg);
    void display(QByteArray msg);
    QByteArray crc16(QByteArray msg);
    virtual void showEvent(QShowEvent *e);
    virtual void hideEvent(QHideEvent *e);
private:
    QHBoxLayout *toplayout;
    QHBoxLayout *lowlayout;
    QTextEdit *textRecv;
    QCheckBox *boxSpace;
    QCheckBox *boxHex16;
    QCheckBox *boxCrc16;
    QCheckBox *boxEnter;
    QList<QComboBox*> boxRecver;
    QList<QLineEdit*> boxSender;
    QList<QPushButton*> btnSender;
    QList<QLineEdit*> boxReturn;
    QList<QLineEdit*> boxDelays;
    QMap<int, int> wait;
    QMap<int, int> isRecv;

    QSerialPort *com;
    QTimer *timer;
};

#endif // APPSERIAL_H

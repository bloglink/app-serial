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
#include <QFileDialog>

#include <QDateTime>
#include <QListView>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QSettings>
#include <QTableView>
#include <QHideEvent>
#include <QPushButton>
#include <QMainWindow>
#include <QApplication>
#include <QFontDatabase>
#include <QHeaderView>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlTableModel>

#include "boxqmodel.h"
#include "boxqitems.h"

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
    void initBoxTop();
    void initBoxMid();
    void initSqlite();
    void initBoxPort();
    void initTxtPort();
    void initBoxSend();
    void initButtons();
    void initSettings();
    void saveSettings();
    void displayExtern();
    void openSerialDev();
    void sendSerialDat();
    void sendSerialMsg();
    void openSerialFile();
    void sendSerialFile();
    void saveSerialFile();
    void sendSerialTab(QModelIndex index);
    void readreadySlot();
    void recvSerialDev();
    QByteArray crc16(QByteArray msg);
    virtual void showEvent(QShowEvent *e);
    virtual void hideEvent(QHideEvent *e);
private:
    QHBoxLayout *toplayout;
    QHBoxLayout *midlayout;
    QHBoxLayout *lowlayout;
    QGridLayout *btnlayout;
    QPushButton *btnOpen;
    QTextEdit *textrecv;
    QTextEdit *textsend;
    QTableView *view;
    BoxQModel *mExtern;
    BoxQModel *mSerial;
    QMap<QString, QComboBox*> combobox;
    QMap<QString, QCheckBox*> checkbox;
    QMap<QString, QLineEdit*> lineedit;
    QVariantMap recver;
    QString stringSend;

    QSerialPort *com;
    QTimer *timerRead;
    QTimer *timerSend;
};

#endif // APPSERIAL_H

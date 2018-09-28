/*******************************************************************************
 * Copyright [2018] <青岛艾普智能仪器有限公司>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       串口调试助手
*******************************************************************************/
#include "appserial.h"

AppSerial::AppSerial(QWidget *parent) : QMainWindow(parent)
{
    initUI();
}

void AppSerial::initUI()
{
    initTitle();
    initLayout();
    initRecvText();
    initRecvPort();
    initRecvItem();
    initSendView();
}

void AppSerial::initTitle()
{
    char s_month[5];
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int month, day, year;
    sscanf((__DATE__), "%s %d %d", s_month, &day, &year);
    month = (strstr(month_names, s_month)-month_names)/3+1;

    static const QDate makeDate(year, month, day);
    static const QTime makeTime = QTime::fromString(__TIME__, "hh:mm:ss");
    static const QDateTime mDateTime(makeDate, makeTime);
    QString ver = QString("V-0.2.%1").arg(mDateTime.toString("yyMMdd-hhmm"));
    this->setWindowTitle(tr("串口调试助手%1 by link").arg(ver));
    qDebug() << "app show:" << ver;
}

void AppSerial::initLayout()
{
    QVBoxLayout *layout = new QVBoxLayout;

    toplayout = new QHBoxLayout;
    layout->addLayout(toplayout);

    lowlayout = new QHBoxLayout;
    layout->addLayout(lowlayout);

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);

    this->setCentralWidget(frame);
    this->resize(800, 600);
}

void AppSerial::initRecvText()
{
    com = NULL;
    textRecv = new QTextEdit(this);
    toplayout->addWidget(textRecv);
}

void AppSerial::initRecvPort()
{
    QStringList names;
    names << tr("串口名") << tr("波特率") << tr("校验位") << tr("数据位") << tr("停止位");

    QGridLayout *layout = new QGridLayout;
    for (int i=0; i < names.size(); i++) {
        layout->addWidget(new QLabel(names.at(i), this), i, 0);
        QComboBox *box = new QComboBox(this);
        layout->addWidget(box, i, 1);
        box->setView(new QListView);
        box->setFixedHeight(35);
        boxRecver.append(box);
    }
    toplayout->addLayout(layout);

    boxSpace = new QCheckBox(tr("空格"), this);
    layout->addWidget(boxSpace,  names.size() + 0x00, 0);
    boxHex16 = new QCheckBox(tr("十六进制"), this);
    layout->addWidget(boxHex16,  names.size() + 0x00, 1);

    boxEnter = new QCheckBox(tr("回车"), this);
    layout->addWidget(boxEnter,  names.size() + 0x01, 0);
    boxCrc16 = new QCheckBox(tr("CRC校验"), this);
    layout->addWidget(boxCrc16,  names.size() + 0x01, 1);

    QPushButton *btnOpen = new QPushButton(tr("打开串口"), this);
    btnOpen->setFixedHeight(44);
    layout->addWidget(btnOpen, names.size() + 2, 0, 1, 2);
    connect(btnOpen, SIGNAL(clicked(bool)), this, SLOT(openSerialDev()));

    QPushButton *btnClear = new QPushButton(tr("清空显示"), this);
    btnClear->setFixedHeight(44);
    layout->addWidget(btnClear, names.size() + 3, 0, 1, 2);
    connect(btnClear, SIGNAL(clicked(bool)), textRecv, SLOT(clear()));
}

void AppSerial::initRecvItem()
{
    QStringList com;
#ifndef __linux__
    QString path = "HKEY_LOCAL_MACHINE\\HARDWARE\\DEVICEMAP\\SERIALCOMM";
    QSettings *settings = new QSettings(path, QSettings::NativeFormat);
    QStringList key = settings->allKeys();
    HKEY hKey;
    int ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
                             0, KEY_READ, &hKey);
    if (ret != 0) {
        qDebug() << "Cannot open regedit!";
    } else {
        for (int i=0; i < key.size(); i++) {
            wchar_t name[256];
            DWORD ikey = sizeof(name);
            char kvalue[256];
            DWORD t = sizeof(kvalue);
            DWORD type;
            QString tmpName;
            ret = ::RegEnumValue(hKey, i, LPWSTR(name), &ikey, 0, &type,
                                 reinterpret_cast<BYTE*>(kvalue), &t);
            if (ret == 0) {
                for (int j = 0; j < static_cast<int>(t); j++) {
                    if (kvalue[j] != 0x00) {
                        tmpName.append(kvalue[j]);
                    }
                }
                com << tmpName;
            }
        }
        RegCloseKey(hKey);
    }
#else
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        com << info.portName();
    }
#endif
    com.sort();
    boxRecver.at(0)->addItems(com);

    QStringList bit;
    bit << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    boxRecver.at(1)->addItems(bit);

    QStringList par;
    par << "None" << "Odd" << "Even";
    boxRecver.at(2)->addItems(par);

    QStringList dat;
    dat << "8" << "7" << "6";
    boxRecver.at(3)->addItems(dat);

    QStringList stop;
    stop << "1" << "1.5" << "2";
    boxRecver.at(4)->addItems(stop);
}

void AppSerial::initSendView()
{
    QGridLayout *layout = new QGridLayout;
    for (int i=0; i < 5; i++) {
        QLineEdit *box = new QLineEdit(this);
        boxSender.append(box);
        box->setFixedHeight(32);
        layout->addWidget(box, i, 0);

        QPushButton *btn = new QPushButton(tr("发送"), this);
        layout->addWidget(btn, i, 1);
        btnSender.append(btn);
        connect(btn, SIGNAL(clicked(bool)), this, SLOT(sendSerialDat()));

        layout->addWidget(new QLabel(tr("自动回复"), this), i, 2);

        QLineEdit *rpy = new QLineEdit(this);
        layout->addWidget(rpy, i, 3);
        rpy->setFixedHeight(32);
        boxReturn.append(rpy);

        layout->addWidget(new QLabel(tr("回复延时"), this), i, 4);

        QLineEdit *dly = new QLineEdit(this);
        layout->addWidget(dly, i, 5);
        dly->setFixedSize(106, 32);
        boxDelays.append(dly);
    }
    lowlayout->addLayout(layout);
}

void AppSerial::initSettings()
{
    QSettings *set = new QSettings("./nandflash/config.ini", QSettings::IniFormat);
    set->setIniCodec("GB18030");
    set->beginGroup("CSerail");

    QStringList item;
    item << set->value("DevPorts", "ttyS0").toString();
    item << set->value("BaudRate", "9600").toString();
    item << set->value("NoParity", "None").toString();
    item << set->value("DataBits", "8").toString();
    item << set->value("StopBits", "1").toString();
    for (int i=0; i < item.size(); i++)
        boxRecver.at(i)->setCurrentText(item.at(i));

    boxSpace->setChecked(set->value("boxSpace", "false").toBool());
    boxHex16->setChecked(set->value("boxHex16", "false").toBool());
    boxCrc16->setChecked(set->value("boxCrc16", "false").toBool());
    boxEnter->setChecked(set->value("boxEnter", "false").toBool());

    for (int i=0; i < boxSender.size(); i++) {
        boxSender.at(i)->setText(set->value(tr("SENDER%1").arg(i)).toString());
        boxReturn.at(i)->setText(set->value(tr("RETURN%1").arg(i)).toString());
        boxDelays.at(i)->setText(set->value(tr("DELAYS%1").arg(i), "100").toString());
    }

    set->deleteLater();
    com = NULL;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendSerialDev()));
}

void AppSerial::saveSettings()
{
    QSettings *set = new QSettings("./nandflash/config.ini", QSettings::IniFormat);
    set->setIniCodec("GB18030");
    set->beginGroup("CSerail");

    set->setValue("DevPorts", boxRecver.at(0)->currentText());
    set->setValue("BaudRate", boxRecver.at(1)->currentText());
    set->setValue("NoParity", boxRecver.at(2)->currentText());
    set->setValue("DataBits", boxRecver.at(3)->currentText());
    set->setValue("StopBits", boxRecver.at(4)->currentText());

    set->setValue("boxSpace", boxSpace->isChecked());
    set->setValue("boxHex16", boxHex16->isChecked());
    set->setValue("boxCrc16", boxCrc16->isChecked());
    set->setValue("boxEnter", boxEnter->isChecked());
    for (int i=0; i < boxSender.size(); i++) {
        set->setValue(tr("SENDER%1").arg(i), boxSender.at(i)->text());
        set->setValue(tr("RETURN%1").arg(i), boxReturn.at(i)->text());
        set->setValue(tr("DELAYS%1").arg(i), boxDelays.at(i)->text());
    }

    set->deleteLater();
}

void AppSerial::openSerialDev()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn->text() == tr("关闭串口")) {
        btn->setText(tr("打开串口"));
        com->close();
        timer->stop();
        return;
    }
    QString name = boxRecver.at(0)->currentText();
    QString bits = boxRecver.at(1)->currentText();
    QString pars = boxRecver.at(2)->currentText();
    QString dats = boxRecver.at(3)->currentText();
    QString stop = boxRecver.at(4)->currentText();
    if (name.contains("COM") && name.size() > 4)
        name.insert(0, "\\\\.\\");
    com = new QSerialPort(name, this);
    if (com->isOpen())
        com->close();
    if (com->open(QIODevice::ReadWrite)) {
        com->setBaudRate(bits.toInt());     //波特率
        if (pars == "None")
            com->setParity(QSerialPort::NoParity);
        if (pars == "Odd")
            com->setParity(QSerialPort::OddParity);
        if (pars == "Even")
            com->setParity(QSerialPort::EvenParity);

        if (dats == "8")
            com->setDataBits(QSerialPort::Data8);
        if (dats == "7")
            com->setDataBits(QSerialPort::Data7);
        if (dats == "6")
            com->setDataBits(QSerialPort::Data6);

        if (stop == "1")
            com->setStopBits(QSerialPort::OneStop);
        if (stop == "1.5")
            com->setStopBits(QSerialPort::OneAndHalfStop);
        if (stop == "2")
            com->setStopBits(QSerialPort::TwoStop);

        com->setFlowControl(QSerialPort::NoFlowControl);
        com->setDataTerminalReady(true);
        com->setRequestToSend(false);
        btn->setText("关闭串口");
        timer->start(1);
        connect(com, SIGNAL(readyRead()), this, SLOT(recvSerialDev()));
        for (int i=0; i < boxSender.size(); i++) {
            isRecv.insert(i, 0);
        }
    } else {
        display(tr("串口打开失败\n").toUtf8());
    }
}

void AppSerial::sendSerialDat()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int index = btnSender.indexOf(btn);
    QString str = boxSender.at(index)->text();
    sendSerialMsg(str.toUtf8());
}

void AppSerial::sendSerialDev()
{
    for (int i=0; i < boxSender.size(); i++) {
        if (isRecv.value(i) > 0) {
            QString str = boxReturn.at(i)->text();
            quint32 ttt = wait.value(i) - 1;
            wait.insert(i, ttt);
            if (ttt == 0) {
                sendSerialMsg(str.toUtf8());
            }
        }
    }
}

void AppSerial::recvSerialDev()
{
    display(com->readAll());
}

void AppSerial::sendSerialMsg(QByteArray msg)
{
    if (com == NULL || !com->isOpen()) {
        textRecv->insertPlainText(tr("请打开串口\n").toUtf8());
        textRecv->moveCursor(QTextCursor::End);
        return;
    }
    if (msg.isEmpty())
        return;
    if (boxHex16->isChecked()) {
        msg.replace(" ", "");
        msg = QByteArray::fromHex(msg);
    }
    if (boxCrc16->isChecked()) {
        msg = crc16(msg);
    }
    com->write(msg);
}

void AppSerial::display(QByteArray msg)
{
    QByteArray hex = msg.toHex().toUpper();
    QByteArray txt = msg;
    if (boxHex16->isChecked() && boxSpace->isChecked()) {
        QStringList tmp;
        for (int i=0; i < hex.size()/2; i++) {
            tmp.append(QString(hex.mid(i*2, 2)));
        }
        txt = QString(tmp.join(" ")).toUtf8();
    }
    txt = (boxHex16->isChecked()) ? txt : msg;
    textRecv->insertPlainText(txt + " ");
    textRecv->moveCursor(QTextCursor::End);

    for (int i=0; i < boxSender.size(); i++) {
        QByteArray str = boxSender.at(i)->text().toUtf8();
        if (boxHex16->isChecked()) {
            str.replace(" ", "");
            str = QByteArray::fromHex(str);
        }
        if (boxCrc16->isChecked()) {
            str = crc16(str);
        }
        if (msg == str) {
            wait.insert(i, boxDelays.at(i)->text().toInt());
            isRecv.insert(i, 1);
        }
    }
}

QByteArray AppSerial::crc16(QByteArray msg)
{
    quint16 crc = 0xFFFF;
    for (int t=0; t < msg.size(); t++) {
        crc = crc ^ quint8(msg.at(t));
        for (int i=0; i < 8; i++) {
            if ((crc & 0x0001) > 0) {
                crc = crc >> 1;
                crc = crc ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    msg.append(crc/256);
    msg.append(crc%256);
    return msg;
}

void AppSerial::showEvent(QShowEvent *e)
{
    initSettings();
    e->accept();
}

void AppSerial::hideEvent(QHideEvent *e)
{
    saveSettings();
    e->accept();
}


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
    initSettings();
}

AppSerial::~AppSerial()
{
}

void AppSerial::initUI()
{
    initSkin();
    initTitle();
    initLayout();
    initRecvBar();
    initSendBar();
}

void AppSerial::initSkin()
{
    QFile file;
    QString qss;
    file.setFileName(":/qss_black.css");
    file.open(QFile::ReadOnly);
    qss = QLatin1String(file.readAll());
    qApp->setStyleSheet(qss);
}

void AppSerial::initTitle()
{
    char s_month[5];
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int month, day, year;

    sscanf((__DATE__), "%s %d %d", s_month, &day, &year);
    month = (strstr(month_names, s_month)-month_names)/3+1;

    QDate dt;
    dt.setDate(year, month, day);
    static const QTime tt = QTime::fromString(__TIME__, "hh:mm:ss");

    QDateTime t(dt, tt);
    QString verNumb = QString("V-0.1.%1").arg(t.toString("yyMMdd-hhmm"));
    this->setWindowTitle(QString("串口调试助手%1").arg(verNumb));
}

void AppSerial::initLayout()
{
    layout = new QGridLayout;
    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);

    this->setCentralWidget(frame);
    this->resize(800, 600);
}

void AppSerial::initRecvBar()
{
    EditGetText = new QTextEdit(this);
    layout->addWidget(EditGetText, 0, 0, 8, 4);
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
    for (int i=0; i < com.size(); i++) {
        if (com.at(i).size() > 4) {
            com.move(i, com.size()-1);
        }
    }
    boxDevPorts = new QComboBox(this);
    boxDevPorts->addItems(com);
    boxDevPorts->setFixedHeight(44);
    boxDevPorts->setView(new QListView);
    layout->addWidget(new QLabel(tr("串口名"), this), 0, 4);
    layout->addWidget(boxDevPorts, 0, 5);

    QStringList bit;
    bit << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    boxBaudRate = new QComboBox(this);
    boxBaudRate->addItems(bit);
    boxBaudRate->setFixedHeight(44);
    boxBaudRate->setView(new QListView);
    layout->addWidget(new QLabel(tr("波特率"), this), 1, 4);
    layout->addWidget(boxBaudRate, 1, 5);

    QStringList par;
    par << "None" << "Odd" << "Even";
    boxNoParity = new QComboBox(this);
    boxNoParity->addItems(par);
    boxNoParity->setFixedHeight(44);
    boxNoParity->setView(new QListView);
    layout->addWidget(new QLabel(tr("校验位"), this), 2, 4);
    layout->addWidget(boxNoParity, 2, 5);

    QStringList dat;
    dat << "8" << "7" << "6";
    boxDataBits = new QComboBox(this);
    boxDataBits->addItems(dat);
    boxDataBits->setFixedHeight(44);
    boxDataBits->setView(new QListView);
    layout->addWidget(new QLabel(tr("数据位"), this), 3, 4);
    layout->addWidget(boxDataBits, 3, 5);

    QStringList stop;
    stop << "1" << "1.5" << "2";
    boxStopBits = new QComboBox(this);
    boxStopBits->addItems(stop);
    boxStopBits->setFixedHeight(44);
    boxStopBits->setView(new QListView);
    layout->addWidget(new QLabel(tr("停止位"), this), 4, 4);
    layout->addWidget(boxStopBits, 4, 5);

    boxAutoSpace = new QCheckBox(tr("空格"), this);
    layout->addWidget(boxAutoSpace, 5, 4);
    boxGetToHEX = new QCheckBox(tr("十六进制接收"), this);
    layout->addWidget(boxGetToHEX, 5, 5);

    QPushButton *btnOpen = new QPushButton(tr("打开串口"), this);
    connect(btnOpen, SIGNAL(clicked(bool)), this, SLOT(openSerialDev()));
    btnOpen->setFixedHeight(44);
    layout->addWidget(btnOpen, 6, 4, 1, 2);

    QPushButton *btnClear = new QPushButton(tr("清空显示"), this);
    connect(btnClear, SIGNAL(clicked(bool)), EditGetText, SLOT(clear()));
    btnClear->setFixedHeight(44);
    layout->addWidget(btnClear, 7, 4, 1, 2);
}

void AppSerial::initSendBar()
{
    EditPutTime = new QLineEdit("1", this);
    EditPutTime->setFixedHeight(44);
    layout->addWidget(new QLabel(tr("发送次数:"), this), 8, 0);
    layout->addWidget(EditPutTime, 8, 1);
    EditPutRate = new QLineEdit("1", this);
    EditPutRate->setFixedHeight(44);
    layout->addWidget(new QLabel(tr("发送间隔(ms):"), this), 8, 2);
    layout->addWidget(EditPutRate, 8, 3);

    boxLoopSend = new QCheckBox(tr("循环"), this);
    layout->addWidget(boxLoopSend, 8, 4);
    boxPutToHEX = new QCheckBox(tr("十六进制发送"), this);
    layout->addWidget(boxPutToHEX, 8, 5);

    EditPutText = new QLineEdit(this);
    EditPutText->setFixedHeight(44);
    layout->addWidget(new QLabel(tr("发送数据:"), this), 9, 0);
    layout->addWidget(EditPutText, 9, 1, 1, 3);

    QPushButton *btnSendCurr = new QPushButton(tr("发送当前"), this);
    connect(btnSendCurr, SIGNAL(clicked(bool)), this, SLOT(sendSerialCurr()));
    btnSendCurr->setFixedHeight(44);
    layout->addWidget(btnSendCurr, 9, 4, 1, 2);

    QStringList headers;
    headers << tr("发送数据列表") << tr("备注");
    tabList = new QTableWidget(this);
    tabList->setColumnCount(2);
    tabList->setHorizontalHeaderLabels(headers);
    tabList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tabList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    layout->addWidget(tabList, 10, 0, 4, 4);
    connect(tabList, SIGNAL(cellClicked(int, int)), this, SLOT(updateCurrentItem(int, int)));

    QPushButton *btnSendList = new QPushButton(tr("发送列表"), this);
    connect(btnSendList, SIGNAL(clicked(bool)), this, SLOT(sendSerialList()));
    btnSendList->setFixedHeight(44);
    layout->addWidget(btnSendList, 10, 4, 1, 2);

    QPushButton *btnStopSend = new QPushButton(tr("停止发送"), this);
    connect(btnStopSend, SIGNAL(clicked(bool)), this, SLOT(stopSerialLoop()));
    btnStopSend->setFixedHeight(44);
    layout->addWidget(btnStopSend, 11, 4, 1, 2);

    QPushButton *btnAppend = new QPushButton(tr("添加任务"), this);
    connect(btnAppend, SIGNAL(clicked(bool)), this, SLOT(appendCurrentItem()));
    btnAppend->setFixedHeight(44);
    layout->addWidget(btnAppend, 12, 4, 1, 2);

    QPushButton *btnDelete = new QPushButton(tr("删除任务"), this);
    connect(btnDelete, SIGNAL(clicked(bool)), this, SLOT(deleteCurrentItem()));
    btnDelete->setFixedHeight(44);
    layout->addWidget(btnDelete, 13, 4, 1, 2);

    QString tmp = tr("串口调试助手 by link");
    layout->addWidget(new QLabel(tmp, this), 14, 0, 1, 4);
}

void AppSerial::initSettings()
{
    QSettings *set = new QSettings("./nandflash/config.ini", QSettings::IniFormat);
    set->setIniCodec("GB18030");
    set->beginGroup("CSerail");
    QString TableItem = "7B 06 00 00 06 7D";
    boxDevPorts->setCurrentText(set->value("DevPorts", "ttyS0").toString());
    boxBaudRate->setCurrentText(set->value("BaudRate", "9600").toString());
    boxNoParity->setCurrentText(set->value("NoParity", "None").toString());
    boxDataBits->setCurrentText(set->value("DataBits", "8").toString());
    boxStopBits->setCurrentText(set->value("StopBits", "1").toString());
    boxAutoSpace->setChecked(set->value("AutoSpace", "false").toBool());
    boxLoopSend->setChecked(set->value("LoopSend", "false").toBool());
    boxGetToHEX->setChecked(set->value("GetToHEX", "false").toBool());
    boxPutToHEX->setChecked(set->value("PutToHEX", "false").toBool());
    EditPutRate->setText(set->value("SendRate", "100").toString());
    EditPutTime->setText(set->value("PutTime", "1").toString());
    EditPutText->setText(set->value("PutText", TableItem).toString());

    tabTexts = QString(set->value("TabTexts", TableItem).toString()).split("@@");
    tabNotes = QString(set->value("TabNotes", "").toString()).split("@@");
    tabList->setRowCount(qMin(tabTexts.size(), tabNotes.size()));
    for (int i=0; i < tabTexts.size(); i++) {
        tabList->setItem(i, 0, new QTableWidgetItem);
        tabList->setItem(i, 1, new QTableWidgetItem);
        tabList->item(i, 0)->setText(tabTexts.at(i));
        tabList->item(i, 1)->setText(tabNotes.at(i));
    }
    set->deleteLater();
    com = NULL;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendSerialDev()));
    stopSerialLoop();
}

void AppSerial::saveSettings()
{
    QSettings *set = new QSettings("./nandflash/config.ini", QSettings::IniFormat);
    set->setIniCodec("GB18030");
    set->beginGroup("CSerail");
    set->setValue("DevPorts", boxDevPorts->currentText());
    set->setValue("BaudRate", boxBaudRate->currentText());
    set->setValue("NoParity", boxNoParity->currentText());
    set->setValue("DataBits", boxDataBits->currentText());
    set->setValue("StopBits", boxStopBits->currentText());
    set->setValue("AutoSpace", boxAutoSpace->isChecked());
    set->setValue("GetToHEX", boxGetToHEX->isChecked());
    set->setValue("LoopSend", boxLoopSend->isChecked());
    set->setValue("PutToHEX", boxPutToHEX->isChecked());
    set->setValue("SendRate", EditPutRate->text());
    set->setValue("PutTime", EditPutTime->text());
    set->setValue("PutText", EditPutText->text());
    for (int i=0; i < tabTexts.size(); i++) {
        tabTexts[i] = tabList->item(i, 0)->text();
    }
    set->setValue("TabTexts", tabTexts.join("@@"));
    for (int i=0; i < tabNotes.size(); i++) {
        tabNotes[i] = tabList->item(i, 1)->text();
    }
    set->setValue("TabNotes", tabNotes.join("@@"));
    set->deleteLater();
}

void AppSerial::openSerialDev()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn->text() == tr("关闭串口")) {
        btn->setText(tr("打开串口"));
        com->close();
        timer->stop();
        stopSerialLoop();
        return;
    }
    QString name = boxDevPorts->currentText();
    if (name.contains("COM") && name.size() > 4)
        name.insert(0, "\\\\.\\");
    if (com == NULL)
        com = new QSerialPort(name, this);
    if (com->isOpen())
        com->close();
    if (com->open(QIODevice::ReadWrite)) {
        com->setBaudRate(boxBaudRate->currentText().toInt());     //波特率
        if (boxNoParity->currentText() == "None")
            com->setParity(QSerialPort::NoParity);
        if (boxNoParity->currentText() == "Odd")
            com->setParity(QSerialPort::OddParity);
        if (boxNoParity->currentText() == "Even")
            com->setParity(QSerialPort::EvenParity);

        if (boxDataBits->currentText() == "8")
            com->setDataBits(QSerialPort::Data8);
        if (boxDataBits->currentText() == "7")
            com->setDataBits(QSerialPort::Data7);
        if (boxDataBits->currentText() == "6")
            com->setDataBits(QSerialPort::Data6);

        if (boxStopBits->currentText() == "1")
            com->setStopBits(QSerialPort::OneStop);
        if (boxStopBits->currentText() == "1.5")
            com->setStopBits(QSerialPort::OneAndHalfStop);
        if (boxStopBits->currentText() == "2")
            com->setStopBits(QSerialPort::TwoStop);

        com->setFlowControl(QSerialPort::NoFlowControl);
        com->setDataTerminalReady(true);
        com->setRequestToSend(false);
        btn->setText("关闭串口");
        timer->start(1);
        connect(com, SIGNAL(readyRead()), this, SLOT(recvSerialDev()));
    } else {
        display(tr("串口打开失败\n").toUtf8());
    }
}

void AppSerial::recvSerialDev()
{
    if (com->bytesAvailable() > 0)
        display(com->readAll());
}

void AppSerial::sendSerialDev()
{
    if (sendOut == 1) {
        timeOut++;
        int t = timeOut / EditPutRate->text().toInt();
        if (t >= EditPutTime->text().toInt() && !boxLoopSend->isChecked()) {
            stopSerialLoop();
        } else if (timeOut % EditPutRate->text().toInt() == 0) {
            sendSerialMsg(EditPutText->text().toUtf8());
        }
    }
    if (sendOut == 2) {
        timeOut++;
        int t = timeOut / EditPutRate->text().toInt();
        int tt = t % tabList->rowCount();
        int ttt = t / tabList->rowCount();
        if (ttt >= EditPutTime->text().toInt() && !boxLoopSend->isChecked()) {
            stopSerialLoop();
        } else if (timeOut % EditPutRate->text().toInt() == 0) {
            sendSerialMsg(tabList->item(tt, 0)->text().toUtf8());
        }
    }
}

void AppSerial::sendSerialCurr()
{
    QString temp = EditPutText->text();
    if (temp.isEmpty())
        return;
    sendOut = 1;
    sendSerialMsg(temp.toUtf8());
}

void AppSerial::sendSerialList()
{
    QString temp = tabList->item(0, 0)->text();
    if (temp.isEmpty())
        return;
    sendOut = 2;
    sendSerialMsg(temp.toUtf8());
}

void AppSerial::stopSerialLoop()
{
    sendOut = 0;
    timeOut = 0;
}

void AppSerial::sendSerialMsg(QByteArray msg)
{
    if (com == NULL || !com->isOpen()) {
        EditGetText->insertPlainText(tr("请打开串口\n").toUtf8());
        EditGetText->moveCursor(QTextCursor::End);
        stopSerialLoop();
        return;
    }
    if (msg.isEmpty())
        return;
    if (boxPutToHEX->isChecked()) {
        msg.replace(" ", "");
        msg = QByteArray::fromHex(msg);
    }
    com->write(msg);
}

void AppSerial::display(QByteArray msg)
{
    QByteArray hex = msg.toHex().toUpper();
    QByteArray temp;
    if (boxGetToHEX->isChecked()) {
        if (boxAutoSpace->isChecked()) {
            for (int i=0; i < hex.size()/2; i++) {
                temp.append(hex.at(2*i));
                temp.append(hex.at(2*i+1));
                temp.append(" ");
            }
        } else {
            temp = hex;
        }
    } else {
        temp = msg;
    }
    EditGetText->insertPlainText(temp);
    EditGetText->moveCursor(QTextCursor::End);
}

void AppSerial::appendCurrentItem()
{
    int t = tabTexts.size();
    tabList->setRowCount(t+1);
    tabList->setItem(t, 0, new QTableWidgetItem);
    tabList->setItem(t, 1, new QTableWidgetItem);
    tabTexts.append(EditPutText->text());
    tabNotes.append("");
    tabList->item(t, 0)->setText(tabTexts.last());
    tabList->item(t, 1)->setText(tabNotes.last());
}

void AppSerial::deleteCurrentItem()
{
    int t = tabList->currentRow();
    if (t < 0)
        return;
    int k = tabList->rowCount();
    tabList->setRowCount(k-1);
    tabTexts.removeAt(t);
    tabNotes.removeAt(t);
    for (int i=0; i < k-1; i++) {
        tabList->item(i, 0)->setText(tabTexts.at(i));
        tabList->item(i, 1)->setText(tabNotes.at(i));
    }
}

void AppSerial::updateCurrentItem(int r, int c)
{
    if (c == 0)
        EditPutText->setText(tabList->item(r, 0)->text());
}

void AppSerial::hideEvent(QHideEvent *e)
{
    saveSettings();
    e->accept();
}


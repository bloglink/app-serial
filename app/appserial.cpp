/*******************************************************************************
 * Copyright [2018] <青岛艾普智能仪器有限公司>
 * All rights reserved.
 *
 * version:     0.3
 * author:      zhaonanlin
 * brief:       串口调试助手
*******************************************************************************/
#include "appserial.h"

#define FIXHEIGHT 30

AppSerial::AppSerial(QWidget *parent) : QMainWindow(parent)
{
    initUI();
}

void AppSerial::initUI()
{
    initTitle();
    initLayout();
    initBoxTop();
    initSqlite();
    initBoxMid();
    initBoxPort();
    initTxtPort();
    initButtons();
    initBoxSend();
}

void AppSerial::initTitle()
{  // 初始化标题栏,根据编译时间自动更新版本号
    char s_month[5];
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    int month, day, year;
    sscanf((__DATE__), "%s %d %d", s_month, &day, &year);
    month = (strstr(month_names, s_month)-month_names)/3+1;

    static const QDate makeDate(year, month, day);
    static const QTime makeTime = QTime::fromString(__TIME__, "hh:mm:ss");
    static const QDateTime mDateTime(makeDate, makeTime);
    QString ver = QString("V-0.3.%1").arg(mDateTime.toString("yyMMdd-hhmm"));
    this->setWindowTitle(tr("串口调试助手%1 by link").arg(ver));
}

void AppSerial::initLayout()
{
    QVBoxLayout *layout = new QVBoxLayout;

    toplayout = new QHBoxLayout;
    layout->addLayout(toplayout);

    midlayout = new QHBoxLayout;
    layout->addLayout(midlayout);

    lowlayout = new QHBoxLayout;
    layout->addLayout(lowlayout);

    layout->setStretch(0, 2);
    layout->setStretch(1, 1);

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);

    this->setCentralWidget(frame);
    this->resize(800, 600);
}

void AppSerial::initBoxTop()
{
    com = NULL;

    textrecv = new QTextEdit(this);
    toplayout->addWidget(textrecv);

    view = new QTableView(this);
    view->verticalHeader()->setVisible(false);              // 隐藏垂直标题栏
    view->horizontalHeader()->setFixedHeight(24);           // 设置标题栏高度
    view->horizontalHeader()->setDefaultSectionSize(35);    // 设置标题栏默认宽度
    view->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    connect(view, SIGNAL(clicked(QModelIndex)), this, SLOT(sendSerialTab(QModelIndex)));
    toplayout->addWidget(view);
    toplayout->setStretch(0, 3);
    toplayout->setStretch(1, 4);

    QFile file("./qrc/fontawesome-webfont.ttf");  // 使用图标字体库
    if (file.exists()) {
        int fontId = QFontDatabase::addApplicationFont("./qrc/fontawesome-webfont.ttf");
        QString fontName = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont iconFont = QFont(fontName);
        view->setFont(iconFont);
    }
    view->hide();
}

void AppSerial::initSqlite()
{
    QFile file("sqlite.db");
    if (!file.exists()) {
        file.open(QIODevice::ReadWrite | QIODevice::Text);  // 数据库文件不存在则自动创建
        file.close();
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "sqlite");
    db.setDatabaseName("sqlite.db");
    if (db.open()) {
        QSqlQuery query(db);
        QString cmd;
        cmd = "create table if not exists serial_config (";
        cmd += "name text primary key, parm text)";
        if (!query.exec(cmd)) {  // 创建配置数据表
            qWarning() << "serial_config:" << query.lastError();
        }
        cmd = "create table if not exists extern_config (";
        cmd += "uuid integer primary key,";
        cmd += "conf_check text, conf_data text, conf_send text, ";
        cmd += "conf_reply text, conf_lenth)";
        if (!query.exec(cmd)) {  // 创建发送列表
            qWarning() << "extern_config:" << query.lastError();
        }
        mSerial = new BoxQModel(this, db);
        mSerial->setTable("serial_config");
        mSerial->select();

        mExtern = new BoxQModel(this, db);
        mExtern->setTable("extern_config");
        mExtern->select();
        if (mExtern->rowCount() == 0) {  // 自动填充发送列表
            for (int i=0; i < 10; i++) {
                query.prepare("insert into extern_config values(?,?,?,?,?,?)");
                query.addBindValue(i);
                query.addBindValue(QChar(0xf10c));
                query.addBindValue("");
                query.addBindValue(QChar(0xf064));
                query.addBindValue("");
                query.addBindValue("5");
                if (!query.exec())
                    qDebug() << query.lastError();
            }
        }
        mExtern->select();

        QStringList titles;
        titles << "ID" << "HEX" << "字符串" << "发送" << "自动回复" << "字长";
        for (int i=0; i < titles.size(); i++)
            mExtern->setHeaderData(i, Qt::Horizontal, titles.at(i));
        for (int i=0; i < mExtern->columnCount(); i++)
            mExtern->setCenter(i);
        view->setModel(mExtern);
        view->hideColumn(0);
        view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
        view->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
        view->setItemDelegateForColumn(1, new BoxQItems);
        view->setItemDelegateForColumn(3, new BoxQItems);
    }
}

void AppSerial::initBoxMid()
{
    QHBoxLayout *layoutfile = new QHBoxLayout;
    QHBoxLayout *layoutshow = new QHBoxLayout;
    midlayout->addLayout(layoutfile);
    midlayout->addLayout(layoutshow);
    midlayout->setStretch(0, 3);
    midlayout->setStretch(1, 4);

    QPushButton *btnOpen = new QPushButton(tr("打开文件"), this);
    btnOpen->setFixedHeight(FIXHEIGHT);
    layoutfile->addWidget(btnOpen);
    connect(btnOpen, SIGNAL(clicked(bool)), this, SLOT(openSerialFile()));

    QLineEdit *file = new QLineEdit(this);
    file->setFixedHeight(FIXHEIGHT);
    layoutfile->addWidget(file);
    lineedit.insert("sendfile", file);

    QPushButton *btnSend = new QPushButton(tr("发送文件"), this);
    btnSend->setFixedHeight(FIXHEIGHT);
    layoutshow->addWidget(btnSend);
    connect(btnSend, SIGNAL(clicked(bool)), this, SLOT(sendSerialFile()));

    QPushButton *btnSave = new QPushButton(tr("保存窗口"), this);
    btnSave->setFixedHeight(FIXHEIGHT);
    layoutshow->addWidget(btnSave);
    connect(btnSave, SIGNAL(clicked(bool)), this, SLOT(saveSerialFile()));

    QPushButton *btnClear = new QPushButton(tr("清空窗口"), this);
    btnClear->setFixedHeight(FIXHEIGHT);
    layoutshow->addWidget(btnClear);
    connect(btnClear, SIGNAL(clicked(bool)), textrecv, SLOT(clear()));

    QCheckBox *btnHex = new QCheckBox(tr("HEX"), this);
    layoutshow->addWidget(btnHex);
    checkbox.insert("checkshow", btnHex);

    QCheckBox *btnTime = new QCheckBox(tr("时间戳"), this);
    layoutshow->addWidget(btnTime);
    checkbox.insert("checktime", btnTime);

    QPushButton *btnMore = new QPushButton(tr("扩展"), this);
    btnMore->setFixedHeight(FIXHEIGHT);
    layoutshow->addWidget(btnMore);
    connect(btnMore, SIGNAL(clicked(bool)), this, SLOT(displayExtern()));
}

void AppSerial::initBoxPort()
{
    QStringList texts;
    texts << tr("串口名") << tr("波特率") << tr("校验位")
          << tr("数据位") << tr("停止位") << tr("流控制");
    QStringList names;
    names << "portname" << "baudrate" << "parities" << "databits" << "stopbits" << "flowctrl";
    btnlayout = new QGridLayout;
    for (int i=0; i < texts.size(); i++) {
        btnlayout->addWidget(new QLabel(texts.at(i), this), i, 0);
        QComboBox *box = new QComboBox(this);
        btnlayout->addWidget(box, i, 1);
        box->setView(new QListView);
        box->setFixedHeight(FIXHEIGHT);
        combobox.insert(names.at(i), box);
    }
    lowlayout->addLayout(btnlayout);
    for (int i=0; i < 4; i++) {
        btnlayout->setColumnStretch(i, (i == 1) ? 5 : (i/2) * 3);
    }
}

void AppSerial::initTxtPort()
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
    combobox.value("portname")->addItems(com);

    QStringList bit;
    bit << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
    combobox.value("baudrate")->addItems(bit);

    QStringList par;
    par << "None" << "Odd" << "Even";
    combobox.value("parities")->addItems(par);

    QStringList dat;
    dat << "8" << "7" << "6";
    combobox.value("databits")->addItems(dat);

    QStringList stop;
    stop << "1" << "1.5" << "2";
    combobox.value("stopbits")->addItems(stop);

    QStringList ctrl;
    ctrl << "None";
    combobox.value("flowctrl")->addItems(ctrl);

    timerRead = new QTimer(this);
    connect(timerRead, SIGNAL(timeout()), this, SLOT(recvSerialDev()));

    timerSend = new QTimer(this);
    connect(timerSend, SIGNAL(timeout()), this, SLOT(sendSerialMsg()));
}

void AppSerial::initButtons()
{
    btnOpen = new QPushButton(tr("打开串口"), this);
    btnOpen->setFixedHeight(FIXHEIGHT);
    btnlayout->addWidget(btnOpen, 0, 2, 1, 2);
    connect(btnOpen, SIGNAL(clicked(bool)), this, SLOT(openSerialDev()));

    QPushButton *btnSend = new QPushButton(tr("发送数据"), this);
    btnSend->setFixedHeight(FIXHEIGHT);
    btnlayout->addWidget(btnSend, 1, 2, 1, 2);
    connect(btnSend, SIGNAL(clicked(bool)), this, SLOT(sendSerialDat()));

    QStringList texts;
    texts << "DTR" << "RST" << "HEX" << tr("换行") << tr("定时") << "CRC";
    QStringList names;
    names << "checkdtr" << "checkrst" << "checkhex" << "checknew" << "checkfix" << "checkcrc";
    for (int i=0; i < texts.size(); i++) {
        QCheckBox *box = new QCheckBox(texts.at(i), this);
        btnlayout->addWidget(box, 2 + i/2, i%2 + 2);
        checkbox.insert(names.at(i), box);
        box->setFixedHeight(FIXHEIGHT);
    }

    QLineEdit *looptime = new QLineEdit("1000", this);
    looptime->setFixedHeight(FIXHEIGHT);
    btnlayout->addWidget(looptime, 5, 2);
    lineedit.insert("looptime", looptime);

    QString strSW = QString("<p align='left'>%1</p>").arg("ms");
    btnlayout->addWidget(new QLabel(strSW, this), 5, 3);
}

void AppSerial::initBoxSend()
{
    textsend = new QTextEdit(this);
    lowlayout->addWidget(textsend);
    lowlayout->setStretch(0, 3);
    lowlayout->setStretch(1, 4);
}

void AppSerial::initSettings()
{
    foreach(QString name, combobox.keys()) {
        for (int i=0; i < mSerial->rowCount(); i++) {
            if (name == mSerial->index(i, 0).data().toString()) {
                QString index = mSerial->index(i, 1).data().toString();
                combobox.value(name)->setCurrentText(index);
            }
        }
    }
    foreach(QString name, checkbox.keys()) {
        for (int i=0; i < mSerial->rowCount(); i++) {
            if (name == mSerial->index(i, 0).data().toString()) {
                int check = mSerial->index(i, 1).data().toInt();
                checkbox.value(name)->setChecked(check == 0 ? false : true);
            }
        }
    }
    foreach(QString name, lineedit.keys()) {
        for (int i=0; i < mSerial->rowCount(); i++) {
            if (name == mSerial->index(i, 0).data().toString()) {
                QString index = mSerial->index(i, 1).data().toString();
                lineedit.value(name)->setText(index);
            }
        }
    }
    if (1) {
        for (int i=0; i < mSerial->rowCount(); i++) {
            if ("textsend" == mSerial->index(i, 0).data().toString()) {
                QString index = mSerial->index(i, 1).data().toString();
                textsend->setText(index);
            }
        }
    }
}

void AppSerial::saveSettings()
{
    QSqlQuery query(QSqlDatabase::database("sqlite"));
    foreach(QString name, combobox.keys()) {
        query.prepare("replace into serial_config values(?,?)");
        query.addBindValue(name);
        query.addBindValue(combobox.value(name)->currentText());
        if (!query.exec())
            qDebug() << query.lastError();
    }
    foreach(QString name, checkbox.keys()) {
        query.prepare("replace into serial_config values(?,?)");
        query.addBindValue(name);
        query.addBindValue(checkbox.value(name)->isChecked() ? 1 : 0);
        if (!query.exec())
            qDebug() << query.lastError();
    }
    foreach(QString name, lineedit.keys()) {
        query.prepare("replace into serial_config values(?,?)");
        query.addBindValue(name);
        query.addBindValue(lineedit.value(name)->text());
        if (!query.exec())
            qDebug() << query.lastError();
    }
    if (1) {
        query.prepare("replace into serial_config values(?,?)");
        query.addBindValue("textsend");
        query.addBindValue(textsend->toPlainText());
        if (!query.exec())
            qDebug() << query.lastError();
    }
    mExtern->submitAll();
}

void AppSerial::displayExtern()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    view->setVisible(btn->text() == tr("扩展"));
    btn->setText(btn->text() == tr("扩展") ? tr("隐藏") : tr("扩展"));
}

void AppSerial::openSerialDev()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn->text() == tr("关闭串口")) {
        btn->setText(tr("打开串口"));
        timerSend->stop();
        com->close();
        return;
    }
    QString portname = combobox.value("portname")->currentText();
    QString baudrate = combobox.value("baudrate")->currentText();
    QString parities = combobox.value("parities")->currentText();
    QString databits = combobox.value("databits")->currentText();
    QString stopbits = combobox.value("stopbits")->currentText();
    if (portname.isEmpty())
        return;
    if (portname.contains("COM") && portname.size() > 4)
        portname.insert(0, "\\\\.\\");
    com = new QSerialPort(portname, this);
    if (com->isOpen())
        com->close();
    if (!com->open(QIODevice::ReadWrite)) {
        QMessageBox::warning(this, "", tr("%1打开失败").arg(portname), QMessageBox::Ok);
        return;
    }
    com->setBaudRate(baudrate.toInt());     //波特率
    if (parities == "None")
        com->setParity(QSerialPort::NoParity);  // 校验位
    if (parities == "Odd")
        com->setParity(QSerialPort::OddParity);
    if (parities == "Even")
        com->setParity(QSerialPort::EvenParity);

    if (databits == "8")
        com->setDataBits(QSerialPort::Data8);  // 数据位
    if (databits == "7")
        com->setDataBits(QSerialPort::Data7);
    if (databits == "6")
        com->setDataBits(QSerialPort::Data6);

    if (stopbits == "1")
        com->setStopBits(QSerialPort::OneStop);  // 停止位
    if (stopbits == "1.5")
        com->setStopBits(QSerialPort::OneAndHalfStop);
    if (stopbits == "2")
        com->setStopBits(QSerialPort::TwoStop);

    com->setFlowControl(QSerialPort::NoFlowControl);  // 流控制
    com->setDataTerminalReady(checkbox.value("checkdtr")->isChecked());
    com->setRequestToSend(checkbox.value("checkrst")->isChecked());
    btn->setText("关闭串口");
    connect(com, SIGNAL(readyRead()), this, SLOT(readreadySlot()));
}

void AppSerial::sendSerialDat()
{
    stringSend = textsend->toPlainText();
    sendSerialMsg();
}

void AppSerial::sendSerialMsg()
{
    if (com == NULL || !com->isOpen()) {
        btnOpen->click();
    }
    if (checkbox.value("checkfix")->isChecked()) {
        int t = lineedit.value("looptime")->text().toInt();
        timerSend->start(t);
    } else {
        timerSend->stop();
    }
    QString dat = stringSend;
    QByteArray msg = dat.toUtf8();
    if (checkbox.value("checkhex")->isChecked())
        msg = QByteArray::fromHex(msg);
    if (checkbox.value("checknew")->isChecked())
        msg.append(0x0D);
    if (checkbox.value("checkcrc")->isChecked())
        msg = crc16(msg);
    com->write(msg);
}

void AppSerial::openSerialFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开文件"),
                                                    "/",
                                                    tr("文本文件(*.txt)"));
    if (fileName.length() != 0) {
        lineedit.value("sendfile")->setText(fileName);
    }
}

void AppSerial::sendSerialFile()
{
    QString fileName = lineedit.value("sendfile")->text();
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray msg = file.readAll();
            if (com == NULL || !com->isOpen()) {
                btnOpen->click();
            }
            com->write(msg);
        }
    }
}

void AppSerial::saveSerialFile()
{
    QString name = QDateTime::currentDateTime().toString("yyMMddhhmmss");
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("保存文件"),
                                                    name,
                                                    tr("(*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray msg = textrecv->toPlainText().toUtf8();
            file.write(msg);
            file.close();
        }
    }
}

void AppSerial::sendSerialTab(QModelIndex index)
{
    if (index.column() == 1) {
        QVariant dat = QChar(0xF192);
        QChar tmp = (index.data().toString() == dat.toString()) ? QChar(0xF10C) : QChar(0xF192);
        mExtern->setData(index, tmp, Qt::EditRole);
    }
    if (index.column() == 3) {
        QString str = mExtern->data(mExtern->index(index.row(), 2), Qt::DisplayRole).toString();
        stringSend = str;
        sendSerialMsg();
    }
}

void AppSerial::readreadySlot()
{
    timerRead->start(10);
}

void AppSerial::recvSerialDev()
{
    timerRead->stop();
    QByteArray msg;
    QByteArray dat = com->readAll();
    if (checkbox.value("checktime")->isChecked()) {
        msg.append("[" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
        msg.append("]");
        msg.append(":");
    }
    msg.append(checkbox.value("checkshow")->isChecked() ? dat.toHex().toUpper() : dat);
    msg.append("\n");
    textrecv->insertPlainText(msg);
    textrecv->moveCursor(QTextCursor::End);
    for (int i=0; i < mExtern->rowCount(); i++) {
        QVariant var = QChar(0xF192);
        if (mExtern->data(mExtern->index(i, 1), Qt::DisplayRole).toString() == var.toString()) {
            QString tmp = mExtern->data(mExtern->index(i, 2), Qt::DisplayRole).toString();
            QString txt = mExtern->data(mExtern->index(i, 4), Qt::DisplayRole).toString();
            QString len = mExtern->data(mExtern->index(i, 5), Qt::DisplayRole).toString();
            if (tmp.startsWith(dat.mid(0, len.toInt()))) {
                stringSend = txt;
                sendSerialMsg();
                break;
            }
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
    QTimer::singleShot(50, btnOpen, SLOT(click()));
    e->accept();
}

void AppSerial::hideEvent(QHideEvent *e)
{
    saveSettings();
    e->accept();
}


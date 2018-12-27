/*******************************************************************************
 * Copyright [2018] <zhaonanlin>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       数据库委托
*******************************************************************************/
#ifndef BOXQMODEL_H
#define BOXQMODEL_H

#include <QDebug>
#include <QObject>
#include <QSqlTableModel>

class BoxQModel : public QSqlTableModel
{
    Q_OBJECT
signals:
    void dataChaged(int index, int row);
public:
    explicit BoxQModel(QObject *parent = 0);
    explicit BoxQModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void setCheckable(int column);
    void setCenter(int column);
private:
    QList<int> showCheck;
    QList<int> showCenter;
    QMap<int, Qt::CheckState> checks;
};

#endif // BOXQMODEL_H

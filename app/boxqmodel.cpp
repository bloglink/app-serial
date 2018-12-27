/*******************************************************************************
 * Copyright [2018] <zhaonanlin>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       数据库委托
*******************************************************************************/
#include "boxqmodel.h"

BoxQModel::BoxQModel(QObject * parent, QSqlDatabase db) : QSqlTableModel(parent, db)
{
}

bool BoxQModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    if (role == Qt::CheckStateRole && showCheck.contains(index.column())) {
        checks[index.row()] = (value == Qt::Checked ? Qt::Checked : Qt::Unchecked);
        return true;
    } else {
        return QSqlTableModel::setData(index, value, role);
    }
}

QVariant BoxQModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
    case Qt::CheckStateRole: {
        if (showCheck.contains(index.column())) {
            if (checks.contains(index.row())) {
                return checks[index.row()] == Qt::Checked ? Qt::Checked : Qt::Unchecked;
            }
            return Qt::Unchecked;
        }
        break;
    }
    case Qt::TextAlignmentRole:
        if (showCenter.contains(index.column()))
            return Qt::AlignCenter;
        break;
    default:
        break;
    }
    return QSqlTableModel::data(index, role);
}

Qt::ItemFlags BoxQModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if (showCheck.contains(index.column()))
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    else
        return QSqlTableModel::flags(index);
    return  Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void BoxQModel::setCheckable(int column)
{
    showCheck.append(column);
}

void BoxQModel::setCenter(int column)
{
    showCenter.append(column);
}

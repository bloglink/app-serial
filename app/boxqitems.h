/*******************************************************************************
 * Copyright [2018] <zhaonanlin>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       只读代理
*******************************************************************************/
#ifndef BOXQITEMS_H
#define BOXQITEMS_H

#include <QObject>
#include <QItemDelegate>

class BoxQItems : public QItemDelegate
{
    Q_OBJECT
public:
    explicit BoxQItems(QObject *parent = 0);

    QWidget *createEditor(QWidget*, const QStyleOptionViewItem &,
                          const QModelIndex &) const;
};

#endif // BOXQITEMS_H

/*******************************************************************************
 * Copyright [2018] <zhaonanlin>
 * All rights reserved.
 *
 * version:     0.1
 * author:      zhaonanlin
 * brief:       只读代理
*******************************************************************************/
#include "boxqitems.h"

BoxQItems::BoxQItems(QObject *parent) : QItemDelegate(parent)
{
}

QWidget *BoxQItems::createEditor(QWidget *,
                                      const QStyleOptionViewItem &,
                                      const QModelIndex &) const {
    return NULL;
}

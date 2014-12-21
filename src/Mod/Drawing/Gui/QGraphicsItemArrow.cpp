/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"
#ifndef _PreComp_
#include <assert.h>
#include <QGraphicsScene>
#include <QMenu>
#include <QMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QStyleOptionGraphicsItem>
#include <QPainterPathStroker>
#include <QPainter>
#endif

#include "QGraphicsItemView.h"
#include "QGraphicsItemArrow.h"

using namespace DrawingGui;

QGraphicsItemArrow::QGraphicsItemArrow(QGraphicsScene *scene)
{
    isFlipped = false;

    if(scene) {
        scene->addItem(this);
    }

    // Set Cache Mode
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

}

QPainterPath QGraphicsItemArrow::shape() const
{
    return this->path();
}

void QGraphicsItemArrow::setHighlighted(bool state)
{
    QPen pen     = this->pen();
    QBrush brush = this->brush();
    if(state) {
        pen.setColor(Qt::blue);
        brush.setColor(Qt::blue);
    } else {
        pen.setColor(Qt::black);
        brush.setColor(Qt::black);
    }
    this->setBrush(brush);
    this->setPen(pen);
}

QVariant QGraphicsItemArrow::itemChange(GraphicsItemChange change, const QVariant &value)
{
//     if (change == ItemSelectedHasChanged && scene()) {
//         // value is the new position.
//         if(isSelected()) {
    //             QBrush brush = this->bru
//     this->setScale(10);sh();
//             brush.setColor(Qt::blue);
//             this->setBrush(brush);
//         } else {
//             QBrush brush = this->brush();
//             brush.setColor(Qt::black);
//             this->setBrush(brush);
//         }
//         update();
//     }
    return QGraphicsPathItem::itemChange(change, value);
}

void QGraphicsItemArrow::flip(bool state) {
    isFlipped = state;
}

void QGraphicsItemArrow::draw() {
    // the center is the end point on a dimension
    QPainterPath path;
    QPen pen(QColor(Qt::black));
    pen.setWidth(1);

    QBrush brush(QColor(Qt::black));
    //this->setPen(pen);
    this->setBrush(brush);

    float length = -5.;

    if(isFlipped)
        length *= -1;
    path.moveTo(QPointF(0.,0.));
    path.lineTo(QPointF(length,-0.6));
    path.lineTo(QPointF(length, 0.6));

    path.closeSubpath();
//     path.moveTo(QPointF(-1,1));
//     path.lineTo(QPointF(1,-1));
    this->setPath(path);

}

void QGraphicsItemArrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;
    QGraphicsPathItem::paint(painter, &myOption, widget);
}

/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                                                                         *
 *   This file is Drawing of the FreeCAD CAx development system.           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A DrawingICULAR PURPOSE.  See the      *
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
#include "QGraphicsItemVertex.h"

using namespace DrawingGui;

QGraphicsItemVertex::QGraphicsItemVertex(int ref, QGraphicsScene *scene  ) : reference(ref)
{
    if(scene) {
        scene->addItem(this);
    }

    // Set Cache Mode
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    setFlag(ItemIgnoresTransformations);
    setAcceptHoverEvents(true);
}

QPainterPath QGraphicsItemVertex::shape() const
{
    return this->path();
}

QVariant QGraphicsItemVertex::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        // value is the new position.
        if(isSelected()) {
            QBrush brush = this->brush();
            brush.setColor(Qt::blue);
            this->setBrush(brush);
        } else {
            QBrush brush = this->brush();
            brush.setColor(Qt::black);
            this->setBrush(brush);
        }
        update();
    }
    return QGraphicsItem::itemChange(change, value);
}

void QGraphicsItemVertex::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QBrush brush = this->brush();
    brush.setColor(Qt::blue);
    this->setBrush(brush);
    update();
}

void QGraphicsItemVertex::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());
    assert(view != 0);

    if(!isSelected() && !view->isSelected()) {
        QBrush brush = this->brush();
        brush.setColor(Qt::black);
        this->setBrush(brush);
        update();
    } else {

    }
}

void QGraphicsItemVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;
    QGraphicsPathItem::paint(painter, &myOption, widget);
}
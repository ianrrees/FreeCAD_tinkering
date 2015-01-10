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
# include <assert.h>
# include <QApplication>
# include <QContextMenuEvent>
# include <QGraphicsScene>
# include <QGraphicsSceneHoverEvent>
# include <QMenu>
# include <QMouseEvent>
# include <QPainter>
# include <QPainterPathStroker>
# include <QStyleOptionGraphicsItem>
#endif

#include <Base/Console.h>

#include <qmath.h>
#include "QGraphicsItemView.h"
#include "QGraphicsItemEdge.h"

using namespace DrawingGui;

QGraphicsItemEdge::QGraphicsItemEdge(int ref, QGraphicsScene *scene) : reference(ref)
{
    if(scene) {
        scene->addItem(this);
    }
    if(ref > 0) {
        this->setAcceptHoverEvents(true);
    }

    // Set Cache Mode for QPainter to reduce drawing required
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    strokeWidth = 1.;
    sf = 1.;

    isCosmetic    = true;
    showHidden    = false;
    isHighlighted = false;

    hPen.setStyle(Qt::DashLine);
    vPen.setStyle(Qt::SolidLine);

    // In edit mode these should be set cosmetic
    vPen.setCosmetic(isCosmetic);
    hPen.setCosmetic(isCosmetic);
    hPen.setColor(Qt::gray);
}

void QGraphicsItemEdge::setVisiblePath(const QPainterPath &path) {
    prepareGeometryChange();
    this->vPath = path;
    update();
}

void QGraphicsItemEdge::setHiddenPath(const QPainterPath &path) {
    prepareGeometryChange();
    this->hPath = path;
    update();
}

QRectF QGraphicsItemEdge::boundingRect() const
{
    return shape().controlPointRect();
}

bool QGraphicsItemEdge::contains(const QPointF &point) const
{
    return shape().contains(point);
}

QPainterPath QGraphicsItemEdge::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(strokeWidth / sf);

    // Combine paths
    QPainterPath p;
    p.addPath(vPath);

    if(showHidden)
        p.addPath(hPath);

    return stroker.createStroke(p);
}

void QGraphicsItemEdge::setHighlighted(bool state)
{
    isHighlighted = state;
    if(isHighlighted) {
        vPen.setColor(Qt::blue);
        hPen.setColor(Qt::blue);
    } else {
        vPen.setColor(Qt::black);
        hPen.setColor(Qt::gray);
    }
    update();
}

QVariant QGraphicsItemEdge::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        // value is the new position.
        if(isSelected()) {
            vPen.setColor(Qt::blue);
            hPen.setColor(Qt::blue);
        } else {
            vPen.setColor(Qt::black);
            hPen.setColor(Qt::gray);
        }
        update();
    }

    return QGraphicsItem::itemChange(change, value);
}

void QGraphicsItemEdge::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    vPen.setColor(Qt::blue);
    hPen.setColor(Qt::blue);
    update();
}

void QGraphicsItemEdge::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());
    assert(view != 0);

    if(!isSelected() && !isHighlighted) {
        vPen.setColor(Qt::black);
        hPen.setColor(Qt::gray);
        update();
    }
}

void QGraphicsItemEdge::setCosmetic(bool state)
{
    isCosmetic = state;
    vPen.setCosmetic(isCosmetic);
    hPen.setCosmetic(isCosmetic);
    update();
}

void QGraphicsItemEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;

    painter->setPen(vPen);
    painter->setBrush(vBrush);
    painter->drawPath(vPath);

    // hacky method to get scale for shape()
    this->sf = painter->worldTransform().m11();
    if(showHidden) {
        painter->setPen(hPen);
        painter->setBrush(hBrush);
        painter->drawPath(hPath);
    }
}

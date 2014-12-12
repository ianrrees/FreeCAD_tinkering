/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                 2014 wandererfan <WandererFan@gmail.com>                *
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
#include <cmath>
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QGraphicsScene>
#include <QMenu>
#include <QMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QPainterPathStroker>
#include <QPainter>
#include <QString>
#include <QTextOption>
#include <sstream>
#endif

#include <qmath.h>

#include <Base/Console.h>

#include "../App/FeatureViewSymbol.h"
#include "QGraphicsItemViewSymbol.h"

using namespace DrawingGui;

QGraphicsItemViewSymbol::QGraphicsItemViewSymbol(const QPoint &pos, QGraphicsScene *scene) :QGraphicsItemView(pos, scene)
{
    // set flags for QGraphicsItemViewSymbol
    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    this->setFlag(ItemSendsGeometryChanges, true);
    this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    this->setAcceptHoverEvents(true);
    this->setPos(pos);

    m_svgRender = new QSvgRenderer();

    m_svgItem = new QGraphicsSvgItem();
    this->addToGroup(m_svgItem);
    
    m_pen.setColor(QColor(0,0,255));       //blue for editting.  sb a preference?
    m_pen.setStyle(Qt::DashLine);

    m_borderItem = new QGraphicsRectItem();
    m_borderItem->setPen(m_pen);
    m_borderItem->setVisible(false);
    this->addToGroup(m_borderItem);
}

QGraphicsItemViewSymbol::~QGraphicsItemViewSymbol()
{
    this->removeFromGroup(m_svgItem);
    delete(m_svgRender);
    delete(m_svgItem);
    this->removeFromGroup(m_borderItem);
    delete(m_borderItem);
}

void QGraphicsItemViewSymbol::updateView(bool update)
{
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewSymbol::getClassTypeId()))
        return;

    // get Feature corresponding to this View
    Drawing::FeatureViewSymbol *viewSymbol = dynamic_cast<Drawing::FeatureViewSymbol *>(this->getViewObject());

    // get Symbol property into SVG item
    QString qs(QString::fromUtf8(viewSymbol->Symbol.getValue()));
    QByteArray qba;
    qba.append(qs);
    if (!load(&qba)) {
        Base::Console().Error("QGraphicsItemViewSymbol::updateView - Could not load %s.Symbol into renderer\n", viewSymbol->getNameInDocument());
    }
    
    //make the border fit svgItem
    m_borderItem->setRect(m_svgItem->boundingRect());

    if(update) {
        QGraphicsItemView::updateView(true);
    } else {
        QGraphicsItemView::updateView();
    }
}
void QGraphicsItemViewSymbol::draw()
{
}
void QGraphicsItemViewSymbol::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsItemView::paint(painter, option, widget);
}

QVariant QGraphicsItemViewSymbol::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            Q_EMIT selected(true);
        } else {
            Q_EMIT selected(false);
        }
        update();
    } else if(change == ItemPositionHasChanged && scene()) {
        updatePos();
        Q_EMIT dragging();
    }

    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewSymbol::setPosition(const double &x, const double &y)
{
    // Set the actual QT Coordinates by setting at qt origin rathern than bbox center
    this->setPos(x -  this->boundingRect().width() / 2., y - this->boundingRect().height() / 2.);
}

void QGraphicsItemViewSymbol::updatePos()
{
    // update Feature's X,Y properties
    Drawing::FeatureViewSymbol *viewSymbol = dynamic_cast<Drawing::FeatureViewSymbol *>(this->getViewObject());
    viewSymbol->X.setValue(this->x());
    viewSymbol->Y.setValue(-1.0 * this->y());
}

bool QGraphicsItemViewSymbol::load(QByteArray *svgBytes)
{
    bool success = m_svgRender->load(*svgBytes);
    m_svgItem->setSharedRenderer(m_svgRender);
    return(success);
}

void QGraphicsItemViewSymbol::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_EMIT hover(true);
    update();
}

void QGraphicsItemViewSymbol::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());
    assert(view != 0);

    Q_EMIT hover(false);
    update();
}

void QGraphicsItemViewSymbol::mousePressEvent( QGraphicsSceneMouseEvent * event)
{
    m_borderItem->setVisible(true);
    QGraphicsItemView::mousePressEvent(event);
    update();
}

void QGraphicsItemViewSymbol::mouseReleaseEvent( QGraphicsSceneMouseEvent * event)
{
    if(scene() && this == scene()->mouseGrabberItem()) {
        Q_EMIT dragFinished();
    }
    m_borderItem->setVisible(false);
    QGraphicsItemView::mouseReleaseEvent(event);
    update();
}

#include "moc_QGraphicsItemViewSymbol.cpp"

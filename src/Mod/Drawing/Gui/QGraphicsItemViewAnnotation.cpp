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

#include "../App/FeatureViewAnnotation.h"
#include "QGraphicsItemViewAnnotation.h"

using namespace DrawingGui;

QGraphicsItemViewAnnotation::QGraphicsItemViewAnnotation(const QPoint &pos, QGraphicsScene *scene) :QGraphicsItemView(pos, scene)
{
    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    this->setFlag(ItemSendsGeometryChanges, true);
    this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    this->setAcceptHoverEvents(true);
    this->setPos(pos);

    m_textItem = new QGraphicsTextItem();
    this->addToGroup(m_textItem);
    m_regColor = QColor(0,0,0);                    // black
    m_hiColor = QColor(0,0,255);                   // blue   sb a preference?
}

QGraphicsItemViewAnnotation::~QGraphicsItemViewAnnotation()
{
    this->removeFromGroup(m_textItem);
    delete(m_textItem);
}

void QGraphicsItemViewAnnotation::draw()
{
}

void QGraphicsItemViewAnnotation::drawAnnotation()
{
}

void QGraphicsItemViewAnnotation::updateView(bool update)
{
    // nothing to display
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewAnnotation::getClassTypeId()))
        return;

    // get Feature corresponding to this View
    Drawing::FeatureViewAnnotation *viewAnno = dynamic_cast<Drawing::FeatureViewAnnotation *>(this->getViewObject());

    // get the Property values
    const std::vector<std::string>& annoText = viewAnno->Text.getValues();
    std::stringstream ss;
    for(std::vector<std::string>::const_iterator it = annoText.begin(); it != annoText.end(); ++it) {
        ss << *it << "\n";
    }
 
    QFont font;
    font.setFamily(QString::fromUtf8(viewAnno->Font.getValue()));
    font.setPointSize(viewAnno->TextSize.getValue());
    m_textItem->setFont(font);

    App::Color c = viewAnno->TextColor.getValue();
    m_regColor.setRgbF(c.r,c.g,c.b,1.0 - c.a);

    QString qs = QString::fromUtf8(ss.str().c_str()); 
    m_textItem->setPlainText(qs);
    m_textItem->adjustSize();

    if(update) {
        QGraphicsItemView::updateView(true);
    } else {
        QGraphicsItemView::updateView();
    }
}

void QGraphicsItemViewAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsItemView::paint(painter, option, widget);
}

QVariant QGraphicsItemViewAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            Q_EMIT selected(true);
            m_textItem->setDefaultTextColor(m_hiColor);
        } else {
            Q_EMIT selected(false);
            m_textItem->setDefaultTextColor(m_regColor);
        }
        update();
    } else if(change == ItemPositionHasChanged && scene()) {
        updatePos();
        Q_EMIT dragging();
    }

    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewAnnotation::setPosition(const double &x, const double &y)
{
    // Set the actual QT Coordinates by setting at qt origin rathern than bbox center
    this->setPos(x -  this->boundingRect().width() / 2., y - this->boundingRect().height() / 2.);
}

void QGraphicsItemViewAnnotation::updatePos()
{
    // update Feature's X,Y after dragging
    Drawing::FeatureViewAnnotation *viewAnno = dynamic_cast<Drawing::FeatureViewAnnotation *>(this->getViewObject());
    viewAnno->X.setValue(this->x());
    viewAnno->Y.setValue(-1.0 * this->y());
}

void QGraphicsItemViewAnnotation::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_EMIT hover(true);
    m_textItem->setDefaultTextColor(m_hiColor);
    update();
}

void QGraphicsItemViewAnnotation::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());
    //assert(view != 0);                 // this fails sometimes. ==> this has no parent?
    if (!view) {
        Base::Console().Log("DEBUG - QGraphicsItemViewAnnotation::hoverLeaveEvent view == 0 (no parent?)\n");
        m_textItem->setDefaultTextColor(m_regColor);
        update();
        return;
    }

    Q_EMIT hover(false);
    if(!isSelected() && !view->isSelected()) {
        m_textItem->setDefaultTextColor(m_regColor);
        update();
    }
}

void QGraphicsItemViewAnnotation::mouseReleaseEvent( QGraphicsSceneMouseEvent * event)
{
    if(scene() && this == scene()->mouseGrabberItem()) {
        Q_EMIT dragFinished();
    }
    m_textItem->setDefaultTextColor(m_regColor);
    QGraphicsItemView::mouseReleaseEvent(event);
    update();
}

#include "moc_QGraphicsItemViewAnnotation.cpp"

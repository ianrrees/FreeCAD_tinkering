/***************************************************************************
 *   Copyright (c) 2012-2013 Luke Parry <l.parry@warwick.ac.uk>            *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      *
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
# include <QAction>
# include <QContextMenuEvent>
# include <QGraphicsScene>
# include <QGraphicsSceneMouseEvent>
# include <QMenu>
# include <QMessageBox>
# include <QMouseEvent>
# include <QPainter>
# include <strstream>
#endif

#include <App/Document.h>
#include <Base/Console.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>

#include "../App/FeatureView.h"
#include "QGraphicsItemView.h"

using namespace DrawingGui;

QGraphicsItemView::QGraphicsItemView(const QPoint &pos, QGraphicsScene *scene)
    :QGraphicsItemGroup(),
     locked(false)
{
    this->setFlags(QGraphicsItem::ItemIsSelectable);
    this->setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    this->setPos(pos);

    //Add object to scene
    scene->addItem(this);
}

QGraphicsItemView::~QGraphicsItemView()
{

}

void QGraphicsItemView::alignTo(QGraphicsItem *item, const QString &alignment)
{
    alignHash.clear();
    alignHash.insert(alignment, item);
}

QVariant QGraphicsItemView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange && scene()) {
        // value is the new position.

        QPointF newPos = value.toPointF();

        if(this->locked){
            newPos.setX(this->pos().x());
            newPos.setY(this->pos().y());
        }

        // TODO  find a better data structure for this
        if(alignHash.size() == 1) {
            QGraphicsItem *item = alignHash.begin().value();
            QString alignMode   = alignHash.begin().key();

            if(alignMode == QString::fromAscii("Vertical")) {
                newPos.setX(item->pos().x());
            } else if(alignMode == QString::fromAscii("Horizontal")) {
                newPos.setY(item->pos().y());
            }
        }
        return newPos;
    }
    return QGraphicsItemGroup::itemChange(change, value);
}

void QGraphicsItemView::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsItem *item = this->parentItem();


  if(this->locked) {
      event->ignore();
  } else {
      QGraphicsItem::mousePressEvent(event);
  }
}

void QGraphicsItemView::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void QGraphicsItemView::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if(scene() && this == scene()->mouseGrabberItem() && !this->locked) {

        double x = this->x(),
               y = this->getY();

        Gui::Command::openCommand("Drag View");
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.X = %f", this->getViewObject()->getNameInDocument(), x);
        Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Y = %f", this->getViewObject()->getNameInDocument(), y);
        Gui::Command::commitCommand();
        //Gui::Command::updateActive();
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void QGraphicsItemView::setPosition(qreal x, qreal y)
{
    this->setPos(x, -y);
}

void QGraphicsItemView::updateView(bool update)
{
    this->setPosition(this->getViewObject()->X.getValue(), this->getViewObject()->Y.getValue());
}

const char * QGraphicsItemView::getViewName() const
{
    return viewName.c_str();
}

Drawing::FeatureView * QGraphicsItemView::getViewObject() const
{
     return viewObj;
}

void QGraphicsItemView::setViewFeature(Drawing::FeatureView *obj)
{
    if(obj == 0)
        return;

    viewObj = obj;

    viewName = obj->getNameInDocument(); //Guarantee access for the name if deleted
    Q_EMIT dirty();
}

#include "moc_QGraphicsItemView.cpp"

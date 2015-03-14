/***************************************************************************
 *   Copyright (c) 2012-2013 Luke Parry <l.parry@warwick.ac.uk>            *
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
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPathStroker>
#include <QStyleOptionGraphicsItem>
#include <QTextOption>
#include <strstream>
#endif

#include <App/Application.h>
#include <App/Document.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>

#include "../App/FeatureView.h"
#include "QGraphicsItemView.h"

using namespace DrawingGui;

QGraphicsItemView::QGraphicsItemView(const QPoint &pos, QGraphicsScene *scene)
    :QGraphicsItemGroup(),
     locked(false),
     borderVisible(true)
{
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setAcceptHoverEvents(true);
    setPos(pos);

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();

    m_colCurrent = m_colNormal;
    m_pen.setColor(m_colCurrent);

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

    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
        m_colCurrent = m_colSel;
        //m_pen.setColor(m_colSel);
        } else {
            m_colCurrent = m_colNormal;
            //m_pen.setColor(m_colNormal);
        }
    }

    return QGraphicsItemGroup::itemChange(change, value);
}

void QGraphicsItemView::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
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
    if(!this->locked) {
        double x = this->x(),
               y = this->getY();
        //TODO: doCommand vs viewobject.X.setValue don't need undo/redo for mouse move
        getViewObject()->X.setValue(x);
        getViewObject()->Y.setValue(y);
        //Gui::Command::openCommand("Drag View");
        //Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.X = %f", this->getViewObject()->getNameInDocument(), x);
        //Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Y = %f", this->getViewObject()->getNameInDocument(), y);
        //Gui::Command::commitCommand();
        //Gui::Command::updateActive();
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void QGraphicsItemView::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // TODO don't like this but only solution at the minute
    if (isSelected()) {
        //Base::Console().Message("TRACE - QGraphicsItemView::hoverEnterEvent - is Selected\n");
        m_colCurrent = m_colSel;
        return;
    } else {
        //Base::Console().Message("TRACE - QGraphicsItemView::hoverEnterEvent - is NOT Selected\n");
        m_colCurrent = m_colPre;
        if(this->shape().contains(event->pos())) {                     // TODO don't like this for determining preselect
            //Base::Console().Message("TRACE - QGraphicsItemView::hoverEnterEvent - shape contains event pos (%f,%f)\n",
            //                        event->pos().x(),event->pos().y());
            m_colCurrent = m_colPre;
        }
    }
    update();
}

void QGraphicsItemView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(isSelected()) {
        m_colCurrent = m_colSel;
    } else {
        m_colCurrent = m_colNormal;
    }
    update();
}

void QGraphicsItemView::setPosition(qreal x, qreal y)
{
    this->setPos(x, -y);
}

void QGraphicsItemView::updateView(bool update)
{
    this->setPosition(this->getViewObject()->X.getValue(), this->getViewObject()->Y.getValue());
    if (update) 
        QGraphicsItem::update(boundingRect());
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
    viewName = obj->getNameInDocument();

    // Set the QGraphicsItemGroup position based on the FeatureView
    float x = obj->X.getValue();
    float y = obj->Y.getValue();
    setPos(x, y);

    Q_EMIT dirty();
}

void QGraphicsItemView::toggleCache(bool state)
{
    this->setCacheMode((state)? NoCache : NoCache);
}

void QGraphicsItemView::drawBorder(QPainter *painter)
{
    //Base::Console().Message("TRACE - QGraphicsItemView::drawBorder - %s, borderVisible: %d\n",
    //                        getViewObject()->Label.getValue(),borderVisible);
    //return;
    
    // Save the current painter state and restore at end
    painter->save();

    // Make a rectangle smaller than the bounding box as a border and draw dashed line for selection
    QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);

    QPen myPen = m_pen;
    myPen.setStyle(Qt::DashLine);
    myPen.setWidth(0.3);
    painter->setPen(myPen);

    // Draw Label
    QString name = QString::fromUtf8(this->getViewObject()->Label.getValue());

    QFont font;                                                          //TODO: font sb param
    font.setFamily(QString::fromAscii("osifont")); // Set to generic sans-serif font
    font.setPointSize(5.f);
    painter->setFont(font);
    QFontMetrics fm(font);

    QPointF pos = box.center();
    pos.setY(box.bottom());
    pos.setX(pos.x() - fm.width(name) / 2.);

    painter->drawText(pos, name);
    painter->drawRect(box);

    painter->restore();
}

void QGraphicsItemView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //QStyleOptionGraphicsItem myOption(*option);
    //myOption.state &= ~QStyle::State_Selected;
    m_pen.setColor(m_colCurrent);

    if(borderVisible){
         this->drawBorder(painter);
    }
    QGraphicsItemGroup::paint(painter, option, widget);
}


#include "moc_QGraphicsItemView.cpp"

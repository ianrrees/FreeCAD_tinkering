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

#include <App/Application.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

#include "../App/FeatureViewAnnotation.h"
#include "QGraphicsItemViewAnnotation.h"

using namespace DrawingGui;

QGraphicsItemViewAnnotation::QGraphicsItemViewAnnotation(const QPoint &pos, QGraphicsScene *scene) 
                            :QGraphicsItemView(pos, scene),
                            borderVisible(true)
{
    setCacheMode(QGraphicsItem::NoCache);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    //setHandlesChildEvents(true);    // Qt says this is now obsolete!! (also doesn't work!!)
    setFiltersChildEvents(true);      // this doesn't work either???
    this->setPos(pos);

    m_textItem = new QGraphicsTextItem();
    this->addToGroup(m_textItem);

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();

    m_colCurrent = m_colNormal;
}

QGraphicsItemViewAnnotation::~QGraphicsItemViewAnnotation()
{
    // m_textItem belongs to this group and will be deleted by parent
}

QVariant QGraphicsItemViewAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            m_colCurrent = m_colSel;
        } else {
            m_colCurrent = m_colNormal;
        }
        update();
    }

    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewAnnotation::setViewAnnoFeature(Drawing::FeatureViewAnnotation *obj)
{
    // called from CanvasView. (once)
    if(obj == 0)
        return;

    this->setViewFeature(static_cast<Drawing::FeatureView *>(obj));
    this->draw();

    // Set the QGraphicsItemGroup Properties based on the FeatureView
    float x = obj->X.getValue();
    float y = obj->Y.getValue();

    this->setPos(x, y);
    Q_EMIT dirty();
}

void QGraphicsItemViewAnnotation::draw()
{
    drawAnnotation();
}

void QGraphicsItemViewAnnotation::drawAnnotation()
{
    // nothing to display
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewAnnotation::getClassTypeId()))
        return;

    // get Feature corresponding to this View
    Drawing::FeatureViewAnnotation *viewAnno = dynamic_cast<Drawing::FeatureViewAnnotation *>(this->getViewObject());

    // get the Text values
    const std::vector<std::string>& annoText = viewAnno->Text.getValues();
    std::stringstream ss;
    for(std::vector<std::string>::const_iterator it = annoText.begin(); it != annoText.end(); ++it) {
        if (it == annoText.begin()) {
            ss << *it;
        } else {
            ss << "\n" << *it ;
        }
    }
 
    QFont font;
    font.setFamily(QString::fromUtf8(viewAnno->Font.getValue()));
    font.setPointSize(viewAnno->TextSize.getValue());
    m_textItem->setFont(font);

    App::Color c = viewAnno->TextColor.getValue();
    m_textItem->setDefaultTextColor(c.asQColor());

    this->prepareGeometryChange();
    QString qs = QString::fromUtf8(ss.str().c_str()); 
    m_textItem->setPlainText(qs);
    m_textItem->adjustSize();
}

void QGraphicsItemViewAnnotation::updateView(bool update)
{
    // nothing to display
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewAnnotation::getClassTypeId()))
        return;

    //QGraphicsItemView::updateView(update);                             // update pos() with Feature X,Y

    // get Feature corresponding to this View
    Drawing::FeatureViewAnnotation *viewAnno = dynamic_cast<Drawing::FeatureViewAnnotation *>(this->getViewObject());

    if(update ||
       viewAnno->isTouched() ||
       viewAnno->Text.isTouched() ||
       viewAnno->Font.isTouched() ||
       viewAnno->TextColor.isTouched() ||
       viewAnno->TextSize.isTouched() ) {
        draw();
    }

    QGraphicsItemView::updateView(update);
}

void QGraphicsItemViewAnnotation::toggleCache(bool state)
{
    this->setCacheMode((state)? NoCache : NoCache);
}

void QGraphicsItemViewAnnotation::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // TODO don't like this but only solution at the minute
    if (isSelected()) {
        return;
    } else {
        if(this->shape().contains(event->pos())) {                     // TODO don't like this for determining preselect
            m_colCurrent = m_colPre;
        }
    }
    update();
}

void QGraphicsItemViewAnnotation::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!isSelected()) {
        m_colCurrent = m_colNormal;
    } else {
        m_colCurrent = m_colSel;
    }
    update();
}

QPainterPath QGraphicsItemViewAnnotation::shape() const {
    QPainterPath path;
    QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);
    path.addRect(box);
    QPainterPathStroker stroker;
    stroker.setWidth(5.f);
    return stroker.createStroke(path);
}

QRectF QGraphicsItemViewAnnotation::boundingRect() const
{
    return m_textItem->boundingRect().adjusted(-3.,-3.,3.,3.);     // bigger than QGraphicsTextItem
}

void QGraphicsItemViewAnnotation::drawBorder(QPainter *painter)
{
  // Save the current painter state and restore at end
  painter->save();

  // Make a rectangle smaller than the bounding box as a border and draw dashed line for selection
  QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);

  QPen myPen;
  myPen.setStyle(Qt::DashLine);
  myPen.setWidth(0.3);
  myPen.setColor(m_colCurrent);
  painter->setPen(myPen);

  // Draw Label
  QString name = QString::fromAscii(this->getViewObject()->Label.getValue());

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

void QGraphicsItemViewAnnotation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;

    if(borderVisible){
         this->drawBorder(painter);
    }
    QGraphicsItemView::paint(painter, option, widget);
}

#include "moc_QGraphicsItemViewAnnotation.cpp"


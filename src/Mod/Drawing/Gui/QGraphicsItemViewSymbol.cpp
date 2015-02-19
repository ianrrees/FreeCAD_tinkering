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
#include <QStyleOptionGraphicsItem>
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

#include "../App/FeatureViewSymbol.h"
#include "QGraphicsItemViewSymbol.h"

using namespace DrawingGui;

QGraphicsItemViewSymbol::QGraphicsItemViewSymbol(const QPoint &pos, QGraphicsScene *scene) :QGraphicsItemView(pos, scene)
{
    setCacheMode(QGraphicsItem::NoCache);
    this->setAcceptHoverEvents(true);
    this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    this->setPos(pos);

    m_svgRender = new QSvgRenderer();

    m_svgItem = new QGraphicsSvgItem();
    this->addToGroup(m_svgItem);
    m_svgItem->setPos(0.,0.);
    
    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();
}

QGraphicsItemViewSymbol::~QGraphicsItemViewSymbol()
{
    // m_svgItem belongs to this group and will be deleted by Qt 
    delete(m_svgRender);
}

QVariant QGraphicsItemViewSymbol::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            m_colCurrent = m_colSel;
            //Q_EMIT selected(true);
        } else {
            m_colCurrent = m_colNormal;
            //Q_EMIT selected(false);
        }
        update();
//    } else if(change == ItemPositionHasChanged && scene()) {
//        updatePos();
//        Q_EMIT dragging();
    }

    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewSymbol::setViewSymbolFeature(Drawing::FeatureViewSymbol *obj)
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
    //Q_EMIT dirty();
}

void QGraphicsItemViewSymbol::updateView(bool update)
{
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewSymbol::getClassTypeId()))
        return;

    // get Feature corresponding to this View
    Drawing::FeatureViewSymbol *viewSymbol = dynamic_cast<Drawing::FeatureViewSymbol *>(this->getViewObject());

    if (update ||
        viewSymbol->isTouched() ||
        viewSymbol->Symbol.isTouched()) {

        draw();
    }

    QGraphicsItemView::updateView(update);
}

void QGraphicsItemViewSymbol::draw()
{
    drawSvg();
}

void QGraphicsItemViewSymbol::drawSvg()
{
    // nothing to display
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewSymbol::getClassTypeId()))
        return;

    // get Feature corresponding to this View
    Drawing::FeatureViewSymbol *viewSymbol = dynamic_cast<Drawing::FeatureViewSymbol *>(this->getViewObject());

    // get file from Symbol property into SVG item
    QString qs(QString::fromUtf8(viewSymbol->Symbol.getValue()));
    QByteArray qba;
    qba.append(qs);
    if (!load(&qba)) {
        Base::Console().Error("QGraphicsItemViewSymbol::drawSvg - Could not load %s.Symbol into renderer\n", viewSymbol->getNameInDocument());
    }
}

void QGraphicsItemViewSymbol::toggleCache(bool state)
{
    this->setCacheMode((state)? NoCache : NoCache);
}

void QGraphicsItemViewSymbol::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // TODO don't like this but only solution at the minute
    if (isSelected()) {
        return;
    } else {
        if(this->shape().contains(event->pos())) {                     // TODO don't like this for determining preselect
            m_colCurrent = m_colPre;
        }
    }
    //Q_EMIT hover(true);
    update();
}

void QGraphicsItemViewSymbol::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!isSelected()) {
        m_colCurrent = m_colNormal;
    } else {
        m_colCurrent = m_colSel;
    }
    //Q_EMIT hover(false);
    update();
}

QPainterPath  QGraphicsItemViewSymbol::shape () const
{
    QPainterPath path;
    QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);
    path.addRect(box);
    QPainterPathStroker stroker;
    stroker.setWidth(5.f);
    return stroker.createStroke(path);
}

QRectF QGraphicsItemViewSymbol::boundingRect() const
{
    return m_svgItem->boundingRect().adjusted(-3.,-3.,3.,3.);     // bigger than QGraphicsSvgItem
}

void QGraphicsItemViewSymbol::drawBorder(QPainter *painter)
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

void QGraphicsItemViewSymbol::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;

    if(borderVisible){
         this->drawBorder(painter);
    }
    QGraphicsItemView::paint(painter, option, widget);
}

bool QGraphicsItemViewSymbol::load(QByteArray *svgBytes)
{
    bool success = m_svgRender->load(*svgBytes);
    m_svgItem->setSharedRenderer(m_svgRender);
    return(success);
}

#include "moc_QGraphicsItemViewSymbol.cpp"


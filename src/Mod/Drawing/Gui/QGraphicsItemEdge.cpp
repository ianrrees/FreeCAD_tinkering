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

#include <App/Application.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

#include <qmath.h>
#include "QGraphicsItemView.h"
#include "QGraphicsItemEdge.h"

using namespace DrawingGui;

QGraphicsItemEdge::QGraphicsItemEdge(int ref, QGraphicsScene *scene) :
    reference(ref)
{
    if(scene) {
        scene->addItem(this);
    }
    if(ref > 0) {
        this->setAcceptHoverEvents(true);
    }

    // Set Cache Mode for QPainter to reduce drawing required
    setCacheMode(QGraphicsItem::NoCache);

    strokeWidth = 1.;
    sf = 1.;

    isCosmetic    = true;
    showHidden    = false;
    isHighlighted = false;

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("HiddenColor", 0x08080800));
    m_colHid = fcColor.asQColor();

    hPen.setStyle(Qt::DashLine);
    hPen.setCapStyle(Qt::RoundCap);
    vPen.setStyle(Qt::SolidLine);
    vPen.setCapStyle(Qt::RoundCap);

    // In edit mode these should be set cosmetic
    vPen.setCosmetic(isCosmetic);
    hPen.setCosmetic(isCosmetic);
    setPrettyNormal();
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
        setPrettySel();
    } else {
        setPrettyNormal();
    }
    update();
}

QVariant QGraphicsItemEdge::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            setPrettySel();
        } else {
            setPrettyNormal();
        }
        update();
    }

    return QGraphicsItem::itemChange(change, value);
}

void QGraphicsItemEdge::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setPrettyPre();
    update();
}

void QGraphicsItemEdge::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());
    assert(view != 0);

    if(!isSelected() && !isHighlighted) {
        setPrettyNormal();
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
    painter->drawPath(vPath);

    // hacky method to get scale for shape()
    this->sf = painter->worldTransform().m11();
    if(showHidden) {
        painter->setPen(hPen);
        painter->setBrush(hBrush);
        painter->drawPath(hPath);
    }
}

void QGraphicsItemEdge::setPrettyNormal() {
    vPen.setColor(m_colNormal);
    hPen.setColor(m_colHid);
}

void QGraphicsItemEdge::setPrettyPre() {
    vPen.setColor(m_colPre);
    hPen.setColor(m_colPre);
}

void QGraphicsItemEdge::setPrettySel() {
    vPen.setColor(m_colSel);
    hPen.setColor(m_colSel);
}


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
#include <QGraphicsSceneHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#endif

#include <App/Application.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

#include <qmath.h>
#include "QGraphicsItemView.h"
#include "QGraphicsItemEdge.h"

using namespace DrawingGui;

QGraphicsItemEdge::QGraphicsItemEdge(int ref) :
    reference(ref)
{
    setCacheMode(QGraphicsItem::NoCache);

    strokeWidth = 1.;

    isCosmetic    = false;
    isHighlighted = false;
    //TODO: investigate if an Edge can be both Hidden and Smooth???
    isHiddenEdge = false;
    isSmoothEdge = false;

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

    m_pen.setStyle(Qt::SolidLine);
    m_pen.setCapStyle(Qt::RoundCap);

    m_pen.setCosmetic(isCosmetic);
    setPrettyNormal();
}

QRectF QGraphicsItemEdge::boundingRect() const
{
    return shape().controlPointRect().adjusted(-2.,-2.,2.,2.);         //a bit bigger than the controlPointRect - for ease of selecting?
}

QVariant QGraphicsItemEdge::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            setPrettySel();
        } else {
            setPrettyNormal();
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void QGraphicsItemEdge::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setPrettyPre();
}

void QGraphicsItemEdge::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());    //this is temp for debug??
    assert(view != 0);

    if(!isSelected() && !isHighlighted) {
        setPrettyNormal();
    }
}

void QGraphicsItemEdge::setCosmetic(bool state)
{
    m_pen.setCosmetic(state);
    update();
}

void QGraphicsItemEdge::setHighlighted(bool b)
{
    isHighlighted = b;
    if(isHighlighted) {
        setPrettySel();
    } else {
        setPrettyNormal();
    }
}

void QGraphicsItemEdge::setPrettyNormal() {
    if (isHiddenEdge) {
        m_colCurrent = m_colHid;
    } else {
        m_colCurrent = m_colNormal;
    }
    update();
}

void QGraphicsItemEdge::setPrettyPre() {
    m_colCurrent = m_colPre;
    update();
}

void QGraphicsItemEdge::setPrettySel() {
    m_colCurrent = m_colSel;
    update();
}

void QGraphicsItemEdge::setStrokeWidth(float width) {
    strokeWidth = width;
    update();
}

void QGraphicsItemEdge::setHiddenEdge(bool b) {
    isHiddenEdge = b;
    if (b) m_colCurrent = m_colHid;
    update();
    //TODO: need some fiddling here so hidden edges don't get selected?? is it ok to select a hidden edge?
}

void QGraphicsItemEdge::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget) {
    m_pen.setWidthF(strokeWidth);
    m_pen.setColor(m_colCurrent);
    setPen(m_pen);
    QGraphicsPathItem::paint (painter, option, widget);
}


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
#include <QMenu>
#include <QMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QStyleOptionGraphicsItem>
#include <QPainterPathStroker>
#include <QPainter>
#endif

#include <App/Application.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

#include "QGraphicsItemView.h"
#include "QGraphicsItemVertex.h"

using namespace DrawingGui;

QGraphicsItemVertex::QGraphicsItemVertex(int ref, QGraphicsScene *scene  ) : 
    reference(ref),
    m_fill(Qt::SolidPattern)
{
    if(scene) {
        scene->addItem(this);
    } else {
        Base::Console().Log("PROBLEM? - QGraphicsItemVertex(%d) has NO scene\n",ref);
    }

    setCacheMode(QGraphicsItem::NoCache);
    setFlag(ItemIgnoresTransformations);
    setAcceptHoverEvents(true);

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();

    m_brush.setStyle(m_fill);
    setPrettyNormal();
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
            setPrettySel();
        } else {
            setPrettyNormal();
        }
        update();
    }
    return QGraphicsItem::itemChange(change, value);
}

void QGraphicsItemVertex::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setPrettyPre();
    update();
}

void QGraphicsItemVertex::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());
    assert(view != 0);

    if(!isSelected() && !view->isSelected()) {
        setPrettyNormal();
        update();
    }
}

void QGraphicsItemVertex::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;
    QGraphicsPathItem::paint(painter, &myOption, widget);
}

void QGraphicsItemVertex::setPrettyNormal() {
    m_pen.setColor(m_colNormal);
    m_brush.setColor(m_colNormal);
    setPen(m_pen);
    setBrush(m_brush);
}

void QGraphicsItemVertex::setPrettyPre() {
    m_pen.setColor(m_colPre);
    m_brush.setColor(m_colPre);
    setPen(m_pen);
    setBrush(m_brush);
}

void QGraphicsItemVertex::setPrettySel() {
    m_pen.setColor(m_colSel);
    m_brush.setColor(m_colSel);
    setPen(m_pen);
    setBrush(m_brush);
}


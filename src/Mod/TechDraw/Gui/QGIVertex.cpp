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
#endif

#include "App/Application.h"
#include "App/Material.h"
#include "Base/Console.h"
#include "Base/Parameter.h"

#include "QGIView.h"
#include "QGIVertex.h"

using namespace TechDrawGui;

QGIVertex::QGIVertex(int index) : 
    GIVertex(index),
    m_colNormal(m_colCurrent) // GIVertex ctor sets m_colCurrent to NormalColor
{
    setCacheMode(QGraphicsItem::NoCache);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setAcceptHoverEvents(true);

    auto hGrp( App::GetApplication().GetUserParameter().GetGroup("BaseApp")->
               GetGroup("Preferences")->GetGroup("Mod/TechDraw/Colors") );

    App::Color fcColor;
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();

    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();

    setPrettyNormal();
}

QVariant QGIVertex::itemChange(GraphicsItemChange change, const QVariant &value)
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

void QGIVertex::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setPrettyPre();
}

void QGIVertex::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGIView *view = dynamic_cast<QGIView *> (parentItem());
    assert(view != 0);

    if(!isSelected() && !isHighlighted) {
        setPrettyNormal();
    }
}

void QGIVertex::setHighlighted(bool b)
{
    isHighlighted = b;
    if(isHighlighted) {
        setPrettySel();
    } else {
        setPrettyNormal();
    }
    update();
}

void QGIVertex::setPrettyNormal() {
    m_colCurrent = m_colNormal;
    update();
}

void QGIVertex::setPrettyPre() {
    m_colCurrent = m_colPre;
    update();
}

void QGIVertex::setPrettySel() {
    m_colCurrent = m_colSel;
    update();
}


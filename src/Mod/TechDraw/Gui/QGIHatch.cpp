/***************************************************************************
 *   Copyright (c) 2015 WandererFan <wandererfan@gmail.com>                *
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
    #include <math.h>
    #include <QGraphicsPathItem>
    #include <QGraphicsTextItem>
    #include <QPainter>
    #include <QString>
#endif


#include "App/Application.h"
#include "App/Material.h"
#include "Base/Parameter.h"

#include "../App/DrawHatch.h"
#include "../App/DrawViewPart.h"

#include "QGIView.h"

#include "QGIHatch.h"

using namespace TechDrawGui;

QGIHatch::QGIHatch(std::string parentHatch) :
    TechDraw::GIHatch(parentHatch)
{
    setHandlesChildEvents(false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::NoCache);

    auto hGrp( App::GetApplication().GetUserParameter().GetGroup("BaseApp")->
               GetGroup("Preferences")->GetGroup("Mod/TechDraw/Colors") );

    App::Color fcColor;
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();

    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();

    setPrettyNormal();
}


QVariant QGIHatch::itemChange(GraphicsItemChange change, const QVariant &value)
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

void QGIHatch::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setPrettyPre();
}

void QGIHatch::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGIView *view = dynamic_cast<QGIView *> (parentItem());

    if(!isSelected() && !view->isSelected()) {
        setPrettyNormal();
    }
}

void QGIHatch::setPrettyNormal()
{
    m_pen.setColor(m_colNormal);
    m_brush.setColor(m_colNormal);
    setPen(m_pen);
    setBrush(m_brush);
}

void QGIHatch::setPrettyPre()
{
    m_pen.setColor(m_colPre);
    m_brush.setColor(m_colPre);
    setPen(m_pen);
    setBrush(m_brush);
}

void QGIHatch::setPrettySel()
{
    m_pen.setColor(m_colSel);
    m_brush.setColor(m_colSel);
    setPen(m_pen);
    setBrush(m_brush);
}


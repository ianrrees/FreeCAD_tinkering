/***************************************************************************
 *   Copyright (c) 2016                    Ian Rees <ian.rees@gmail.com>   *
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
    #include <QStyleOptionGraphicsItem>
#endif // #ifndef _PreComp_

#include "App/Application.h"
#include "App/Material.h"

#include "GIEdge.h"

using namespace TechDraw;

GIEdge::GIEdge(int index) :
    isHiddenEdge(false),
    projIndex(index),
    strokeWidth(1.0)  
{
    m_pen.setStyle(Qt::SolidLine);
    m_pen.setCapStyle(Qt::RoundCap);
    m_pen.setCosmetic(false);

    auto hGrp( App::GetApplication().GetUserParameter().GetGroup("BaseApp")->
               GetGroup("Preferences")->GetGroup("Mod/TechDraw/Colors") );
    
    App::Color fcColor;
    fcColor.setPackedValue( hGrp->GetUnsigned("NormalColor", 0x00000000) );
    m_colCurrent = m_colNormal = m_defNormal = fcColor.asQColor();  // TODO: Do we really need all these?

    fcColor.setPackedValue( hGrp->GetUnsigned("HiddenColor", 0x08080800) );
    m_colHid = fcColor.asQColor();

    hGrp = App::GetApplication().GetUserParameter().GetGroup("BaseApp")->
           GetGroup("Preferences")->GetGroup("Mod/TechDraw");

    m_styleHid = static_cast<Qt::PenStyle> (hGrp->GetInt("HiddenLine", 2));
}


void GIEdge::paint( QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                    QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;

    m_pen.setWidthF(strokeWidth);
    m_pen.setColor(m_colCurrent);
    setPen(m_pen);
    QGraphicsPathItem::paint(painter, &myOption, widget);
}


void GIEdge::setHiddenEdge(bool b) {
    isHiddenEdge = b;
    if (b) {
        m_pen.setStyle(m_styleHid);
        m_colNormal = m_colHid;
    } else {
        m_pen.setStyle(Qt::SolidLine);
        m_colNormal = m_defNormal;
    }
    update();
}


void GIEdge::setStrokeWidth(float width) {
    strokeWidth = width;
    update();
}


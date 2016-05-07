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
    #include <QPen>
    #include <QStyleOptionGraphicsItem>
#endif // #ifndef _PreComp_

#include "App/Application.h"
#include "App/Material.h"

#include "GIVertex.h"

using namespace TechDraw;

GIVertex::GIVertex(int index) :
    projIndex(index),
    m_fill(Qt::SolidPattern),
    m_radius(2.0)
{
    auto hGrp( App::GetApplication().GetUserParameter().GetGroup("BaseApp")->
               GetGroup("Preferences")->GetGroup("Mod/TechDraw/Colors") );

    App::Color fcColor( static_cast<uint32_t>(hGrp->GetUnsigned("NormalColor", 0x00000000)) );
    m_colCurrent = fcColor.asQColor();
}

void GIVertex::paint( QPainter *painter,
                      const QStyleOptionGraphicsItem *option,
                      QWidget *widget )
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;

    QPen pen;
    pen.setColor(m_colCurrent);
    setPen(pen);

    QBrush brush;
    brush.setColor(m_colCurrent);
    brush.setStyle(m_fill);
    setBrush(brush);

    setRect(-m_radius, -m_radius, 2.0 * m_radius, 2.0 * m_radius);

    QGraphicsEllipseItem::paint(painter, &myOption, widget);
}


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

#include "GIFace.h"

using namespace TechDraw;

GIFace::GIFace(int ref) :
    reference(ref)
{
}

void GIFace::paint( QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                    QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    //myOption.state &= ~QStyle::State_Selected;   //temp for debugging

    //m_pen.setColor(m_colCurrent);
    //setPen(m_pen);
    //setBrush(m_brush);
    QGraphicsPathItem::paint(painter, &myOption, widget);
}


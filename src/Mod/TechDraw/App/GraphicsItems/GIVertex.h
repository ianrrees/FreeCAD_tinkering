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

#ifndef GIVERTEX_HEADER
#define GIVERTEX_HEADER

#include <QGraphicsItem>

namespace TechDraw {

/// Draws a vertex as a filled circle on the page.
class TechDrawExport GIVertex : public QGraphicsEllipseItem
{
    public:
        GIVertex(int index);
        virtual ~GIVertex() = default;

        enum {Type = QGraphicsItem::UserType + 105};
        int type() const { return Type;}
        
        virtual void paint( QPainter * painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget = 0 );

    protected:
        /// Index of vertex in Projection. must exist.
        int projIndex;

        /// Current colour of the vertex
        QColor m_colCurrent;

        Qt::BrushStyle m_fill;

        float m_radius;
};  // end class GIVertex

};  // end namespace TechDraw

#endif // #ifndef GIVERTEX_HEADER

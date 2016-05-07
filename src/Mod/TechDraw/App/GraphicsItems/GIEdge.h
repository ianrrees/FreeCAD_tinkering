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

#ifndef GIEDGE_HEADER
#define GIEDGE_HEADER

#include <QGraphicsItem>
#include <QPen>

namespace TechDraw {

/// Draws an edge as a 2D line on the page
class TechDrawExport GIEdge : public QGraphicsPathItem
{
    public:
        GIEdge(int index);
        virtual ~GIEdge() = default;

        enum {Type = QGraphicsItem::UserType + 103};
        int type() const { return Type; }

        virtual void paint( QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget = 0 );

        void setHiddenEdge(bool b);
        bool getHiddenEdge() { return(isHiddenEdge); }

        void setStrokeWidth(float width);

    protected:

        bool isHiddenEdge;
        
        /// Index of edge in Projection. must exist.
        int projIndex;

        /// Hang on to the pen, because QGIEdge makes changes to it
        QPen m_pen;

        /// Current colour of the edge
        QColor m_colCurrent;

        QColor m_colNormal;
        QColor m_colHid;
        QColor m_defNormal;
        Qt::PenStyle m_styleHid;

        float strokeWidth;        
};  // end class GIEdge

};  // end namespace TechDraw

#endif // #ifndef GIEDGE_HEADER


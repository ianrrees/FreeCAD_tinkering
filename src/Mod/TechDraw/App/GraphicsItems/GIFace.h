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

#ifndef GIFACE_HEADER
#define GIFACE_HEADER

#include <QGraphicsItem>

namespace TechDraw {

/// TODO: Finish this up
class TechDrawExport GIFace : public QGraphicsPathItem
{
    public:
        GIFace(int ref);
        virtual ~GIFace() = default;

        enum {Type = QGraphicsItem::UserType + 104};
        int type() const { return Type;}
        virtual void paint( QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget = 0 );

    protected:
        int reference;

};  // end class GIFace

};  // end namespace TechDraw

#endif // #ifndef GIFACE_HEADER


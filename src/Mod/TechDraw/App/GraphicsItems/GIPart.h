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

#ifndef GIPART_HEADER
#define GIPART_HEADER

#include "GIBase.h"
#include "GIVertex.h"
#include "GIEdge.h"
#include "GIFace.h"
#include "GIHatch.h"

namespace TechDraw {

class GIFace;

class TechDrawExport GIPart : virtual public GIBase
{
    public:
        virtual ~GIPart();

        enum {Type = QGraphicsItem::UserType + 102};
        int type() const override { return Type; }

        virtual void updateView(bool update = false) override;

        virtual void draw();

        virtual void tidy();

    protected:
        QVariant graphicsItemChange(GraphicsItemChange change, const QVariant &value) override;

        GIFace * drawFace(TechDrawGeometry::Face *f);

        /// Does the heavy lifting of drawing the part
        QPainterPath drawPainterPath(TechDrawGeometry::BaseGeom *baseGeom) const;

        /// Helper for pathArc()
        /*!
         * x_axis_rotation is in radian
         */
        void pathArcSegment( QPainterPath &path,
                             double xc, double yc,
                             double th0, double th1,
                             double rx, double ry, double xAxisRotation ) const;

        /// Draws an arc using QPainterPath path
        /*!
         * x_axis_rotation is in radian
         */
        void pathArc( QPainterPath &path, double rx, double ry, double x_axis_rotation,
                      bool large_arc_flag, bool sweep_flag,
                      double x, double y,
                      double curx, double cury ) const;

        QList<QGraphicsItem*> deleteItems;

        /// Allows for making a new GIVertex or QGIVertex as required
        virtual GIVertex * makeVertex(int i) const { return new GIVertex(i); }

        /// Allows for making a new GIEdge or QGIEdge as required
        virtual GIEdge * makeEdge(int i) const { return new GIEdge(i); }

        /// Allows for making a new GIFace or QGIFace as required
        virtual GIFace * makeFace(int i) const { return new GIFace(i); }

        /// Allows for making a new GIHatch or QGIHatch as required
        virtual GIHatch * makeHatch(std::string parentHatch) const
            { return new GIHatch(parentHatch); }

};  // end class GIPart

};  // end namespace TechDraw

#endif // #ifndef GIPART_HEADER

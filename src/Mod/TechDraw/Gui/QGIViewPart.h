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

#ifndef DRAWINGGUI_QGRAPHICSITEMVIEWPART_H
#define DRAWINGGUI_QGRAPHICSITEMVIEWPART_H

#include <QObject>
#include <QPainter>

#include "QGIView.h"
#include "QGIFace.h"
#include "QGIEdge.h"
#include "QGIVertex.h"
#include "QGIHatch.h"

#include "../App/Geometry.h"
#include "../App/GraphicsItems/GIPart.h"

namespace TechDraw {
    class DrawViewPart;
    class DrawHatch;
}

namespace TechDrawGui
{

class TechDrawGuiExport QGIViewPart : virtual public QGIView, virtual public TechDraw::GIPart
{
public:
    QGIViewPart();
    ~QGIViewPart() = default;

    void toggleCache(bool state) override;
    void toggleCosmeticLines(bool state);
    void toggleVertices(bool state);

    void draw() override;
    virtual QRectF boundingRect() const override;

protected:
    virtual QVariant guiGraphicsItemChange(GraphicsItemChange change, const QVariant &value) override;

    std::vector <TechDraw::DrawHatch *> getHatchesForView(TechDraw::DrawViewPart* viewPart);

    QColor m_colHid;

    /// Allows for making a new GIVertex or QGIVertex as required
    virtual TechDraw::GIVertex * makeVertex(int i) const override
        { return new QGIVertex(i); }

    /// Allows for making a new GIEdge or QGIEdge as required
    virtual TechDraw::GIEdge * makeEdge(int i) const override
        { return new QGIEdge(i); }

    /// Allows for making a new GIFace or QGIFace as required
    virtual TechDraw::GIFace * makeFace(int i) const override
        { return new QGIFace(i); }

    /// Allows for making a new GIHatch or QGIHatch as required
    virtual TechDraw::GIHatch * makeHatch(std::string parentHatch) const override
        { return new QGIHatch(parentHatch); }
};

} // namespace TechDrawGui

#endif // DRAWINGGUI_QGRAPHICSITEMVIEWPART_H

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
    ~QGIViewPart();

    void toggleCache(bool state) override;
    void toggleCosmeticLines(bool state);
    void toggleVertices(bool state);

    virtual void updateView(bool update = false) override;
    void tidy();

    void draw() override;
    virtual QRectF boundingRect() const override;

protected:

    std::vector <TechDraw::DrawHatch *> getHatchesForView(TechDraw::DrawViewPart* viewPart);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    QColor m_colHid;

private:
    QList<QGraphicsItem*> deleteItems;
};

} // namespace TechDrawGui

#endif // DRAWINGGUI_QGRAPHICSITEMVIEWPART_H

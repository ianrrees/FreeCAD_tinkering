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

#include "QGraphicsItemView.h"
#include "QGraphicsItemFace.h"
#include "QGraphicsItemEdge.h"
#include "QGraphicsItemVertex.h"
#include "QGraphicsItemHatch.h"

namespace Drawing {
class FeatureViewPart;
class FeatureHatch;
}

namespace DrawingGeometry {
class BaseGeom;
}

namespace DrawingGui
{

class DrawingGuiExport QGraphicsItemViewPart : public QGraphicsItemView
{
    Q_OBJECT

public:

    explicit QGraphicsItemViewPart(const QPoint &position, QGraphicsScene *scene);
    ~QGraphicsItemViewPart();

    enum {Type = QGraphicsItem::UserType + 102};
    int type() const { return Type;}


    void toggleCache(bool state);
    void toggleCosmeticLines(bool state);
    void toggleVertices(bool state);
    void setViewPartFeature(Drawing::FeatureViewPart *obj);
    virtual void updateView(bool update = false);
    void tidy();

    virtual void draw();
    virtual QRectF boundingRect() const;

Q_SIGNALS:
    void selected(bool state);
    void dirty();

protected:
    QGraphicsItemEdge * findRefEdge(int i);
    QGraphicsItemVertex * findRefVertex(int idx);

    /// Helper for pathArc()
    /*!
     * x_axis_rotation is in radian
     */
    void pathArcSegment(QPainterPath &path, double xc, double yc, double th0,
                        double th1,double rx, double ry, double xAxisRotation) const;

    /// Draws an arc using QPainterPath path
    /*!
     * x_axis_rotation is in radian
     */
    void pathArc(QPainterPath &path, double rx, double ry, double x_axis_rotation,
                                     bool large_arc_flag, bool sweep_flag,
                                     double x, double y,
                                     double curx, double cury) const;

    QPainterPath drawPainterPath(DrawingGeometry::BaseGeom *baseGeom) const;
    std::vector <Drawing::FeatureHatch *> getHatchesForView(Drawing::FeatureViewPart* viewPart);
    void drawViewPart();

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected:
    QColor m_colHid;

private:
    QList<QGraphicsItem *> deleteItems;
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMVIEWPART_H

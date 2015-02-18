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
#include <QStyleOptionGraphicsItem>

#include "QGraphicsItemView.h"
#include "QGraphicsItemFace.h"
#include "QGraphicsItemEdge.h"
#include "QGraphicsItemVertex.h"

namespace Drawing {
class FeatureViewPart;
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

    void tidy();

    enum {Type = QGraphicsItem::UserType + 102};
    int type() const { return Type;}

    void setViewPartFeature(Drawing::FeatureViewPart *obj);

    void toggleCache(bool state);
    void toggleCosmeticLines(bool state);
    void toggleVertices(bool state);
    void toggleBorder(bool state = true) { this->borderVisible = state; }

    virtual void updateView(bool update = false);
    virtual void draw();

Q_SIGNALS:
    void selected(bool state);
    void dirty();

public:
    virtual QPainterPath  shape () const;
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

protected:
    QGraphicsItemEdge * findRefEdge(int i);
    QGraphicsItemVertex * findRefVertex(int idx);

    QPainterPath drawPainterPath(DrawingGeometry::BaseGeom *baseGeom) const;

    // Helper methods for drawing arc segments
    void pathArcSegment(QPainterPath &path,double xc, double yc, double th0, double th1,double rx, double ry, double xAxisRotation) const;
    void pathArc(QPainterPath &path, double rx, double ry, double x_axis_rotation,
                                     bool large_arc_flag, bool sweep_flag,
                                     double x, double y,
                                     double curx, double cury) const;

    void drawViewPart();
    void drawBorder(QPainter *painter);

    // Selection detection
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

protected:
    QPen pen;
    QPen m_pen;
    QBrush m_brush;
    QColor m_colNormal;
    QColor m_colPre;
    QColor m_colSel;
    QColor m_colHid;
    //QRectF bbox;
    bool borderVisible;
private:
    QList<QGraphicsItem *> deleteItems;
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMVIEWPART_H

/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                                                                         *
 *   This file is Drawing of the FreeCAD CAx development system.           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A DrawingICULAR PURPOSE.  See the      *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef DRAWINGGUI_QGRAPHICSITEMFACE_H
#define DRAWINGGUI_QGRAPHICSITEMFACE_H

#include <QGraphicsItem>

QT_BEGIN_NAMESPACE
class QPainter;
class QStyleOptionGraphicsItem;
QT_END_NAMESPACE

namespace DrawingGeometry {
class BaseGeom;
}

namespace DrawingGui
{

class QGraphicsItemFace : public QGraphicsPathItem
{
public:
    explicit QGraphicsItemFace(int ref = -1, QGraphicsScene *scene = 0 );
    ~QGraphicsItemFace() {}

    enum {Type = QGraphicsItem::UserType + 104};
    int type() const { return Type;}

public:
//      QPainterPath shape() const;
    int getReference() const { return reference; }

protected:
    // Preselection events:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    // Selection detection
     QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected:
int reference;

private:
QPen m_pen;
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMFACE_H
/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef DRAWINGGUI_QGRAPHICSITEMEDGE_H
#define DRAWINGGUI_QGRAPHICSITEMEDGE_H

# include <QGraphicsItem>

QT_BEGIN_NAMESPACE
class QPainter;
class QStyleOptionGraphicsItem;
QT_END_NAMESPACE

namespace DrawingGeometry {
class BaseGeom;
}

namespace DrawingGui
{

class DrawingGuiExport QGraphicsItemEdge : public QGraphicsPathItem
{
public:
    explicit QGraphicsItemEdge(int ref = -1, QGraphicsScene *scene = 0 );
    ~QGraphicsItemEdge() {}

    enum {Type = QGraphicsItem::UserType + 103};

    int type() const { return Type;}

    void setHighlighted(bool state);
    void setShowHidden(bool state) {showHidden = state; update(); }
    void setCosmetic(bool state);

    int getReference() const { return reference; }
    QPainterPath getHiddenPath() { return hPath; }
    QPainterPath getVisiblePath() { return vPath; }

    void setStrokeWidth(float width) { strokeWidth = width; vPen.setWidthF(width); hPen.setWidthF(width * 0.3); update(); }
    void setHiddenPath(const QPainterPath &path);
    void setVisiblePath(const QPainterPath &path);

public:
    bool contains(const QPointF &point) const;
    QPainterPath shape() const;
    QRectF boundingRect() const;
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    // Selection detection
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected:
  int reference;
  QPainterPath hPath;
  QPainterPath vPath;

  QBrush hBrush;
  QBrush vBrush;
  QPen   vPen;
  QPen   hPen;

  float sf;
  bool isHighlighted;
  bool isCosmetic;
  bool showHidden;

private:
float strokeWidth;

};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMEDGE_H
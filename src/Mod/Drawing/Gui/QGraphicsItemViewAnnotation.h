/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                 2014 wandererfan <WandererFan@gmail.com>                *
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

#ifndef DRAWINGGUI_QGRAPHICSITEMVIEWANNOTATION_H
#define DRAWINGGUI_QGRAPHICSITEMVIEWANNOTATION_H

#include <QObject>
#include <QPainter>

#include "QGraphicsItemView.h"

namespace Drawing {
class FeatureViewAnnotation;
}

namespace DrawingGui
{

class DrawingGuiExport QGraphicsItemViewAnnotation : public QGraphicsItemView
{
    Q_OBJECT

public:

    explicit QGraphicsItemViewAnnotation(const QPoint &position, QGraphicsScene *scene);
    ~QGraphicsItemViewAnnotation();

    enum {Type = QGraphicsItem::UserType + 120};      //120??
    int type() const { return Type;}

    virtual void draw();
    void updateView(bool update = false);
    void setViewAnnoFeature(Drawing::FeatureViewAnnotation *obj);
    void toggleCache(bool state);
    //void toggleBorder(bool state = true) { this->borderVisible = state; }
    virtual QPainterPath  shape () const;
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

Q_SIGNALS:
    void hover(bool state);
    void selected(bool state);

protected:
    void drawAnnotation();
    void drawBorder(QPainter *painter);

    // Preselection events:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    // Selection detection
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected:
    QGraphicsTextItem *m_textItem;
    QColor m_colCurrent;
    QColor m_colNormal;
    QColor m_colSel;
    QColor m_colPre;
    //bool borderVisible;
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMVIEWANNOTATION_H

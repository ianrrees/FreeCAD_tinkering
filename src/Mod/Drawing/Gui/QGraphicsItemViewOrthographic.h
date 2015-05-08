/***************************************************************************
 *   Copyright (c) 2012-2013 Luke Parry <l.parry@warwick.ac.uk>            *
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

#ifndef DRAWINGGUI_QGraphicsItemViewOrthographic_H
#define DRAWINGGUI_QGraphicsItemViewOrthographic_H

#include <QGraphicsItemGroup>
#include <QObject>
#include <App/PropertyLinks.h>

#include "QGraphicsItemViewCollection.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QEvent;
QT_END_NAMESPACE

namespace Drawing {
class FeatureView;
}

namespace DrawingGui
{

class DrawingGuiExport QGraphicsItemViewOrthographic : public QGraphicsItemViewCollection
{
    Q_OBJECT

public:
    QGraphicsItemViewOrthographic(const QPoint &position, QGraphicsScene *scene);
    ~QGraphicsItemViewOrthographic();

    enum {Type = QGraphicsItem::UserType + 113};
    int type() const { return Type;}

    void alignTo(QGraphicsItemViewOrthographic *, const QString &alignment);

    virtual void updateView(bool update = false);
    virtual void drawBorder(void);

Q_SIGNALS:
    void dirty();

protected:
    virtual bool sceneEventFilter(QGraphicsItem * watched, QEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    // Mouse handling
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event );
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    QGraphicsItemView * getAnchorQItem() const;

    /// Used for unselecting children while dragging the anchor object
    /*!
     * TODO - Perhaps just pass the mouse event on to the anchor somehow?
     */
    void giveSelectionToAnchor(void) const;

private:
    /// Convenience function
    Drawing::FeatureViewOrthographic * getFeatureView(void) const;

    QGraphicsRectItem * m_backgroundItem;
    QGraphicsItem *origin;
    QPoint mousePos;
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGraphicsItemViewOrthographic_H

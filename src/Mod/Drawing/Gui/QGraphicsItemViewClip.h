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

#ifndef DRAWINGGUI_QGRAPHICSITEMCLIP_H
#define DRAWINGGUI_QGRAPHICSITEMCLIP_H

#include <QObject>
#include <QPainter>

#include "QGraphicsItemView.h"
#include "QGCustomRect.h"
#include "QGCustomClip.h"

namespace Drawing {
class FeatureViewPart;
}

namespace DrawingGui
{

class DrawingGuiExport QGraphicsItemViewClip : public QGraphicsItemView
{
    Q_OBJECT

public:

    explicit QGraphicsItemViewClip(const QPoint &position, QGraphicsScene *scene);
    ~QGraphicsItemViewClip();

    enum {Type = QGraphicsItem::UserType + 123};
    int type() const { return Type;}

    virtual void updateView(bool update = false);

    virtual void draw();
    virtual QRectF boundingRect() const;

Q_SIGNALS:
    void selected(bool state);
    void dirty();

protected:
    void drawClip();
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    QGraphicsItemView* getQGIVByName(std::string name);

private:
    QGCustomRect* m_frame;
    QGCustomClip* m_cliparea;

};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMCLIP_H

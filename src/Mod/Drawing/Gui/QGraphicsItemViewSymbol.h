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

#ifndef DRAWINGGUI_QGRAPHICSITEMVIEWSYMBOL_H
#define DRAWINGGUI_QGRAPHICSITEMVIEWSYMBOL_H

#include <QObject>
#include <QPainter>
#include <QString>
#include <QByteArray>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>

#include "QGraphicsItemView.h"
#include "QGCustomSvg.h"

namespace Drawing {
class FeatureViewSymbol;
}

namespace DrawingGui
{

class DrawingGuiExport QGraphicsItemViewSymbol : public QGraphicsItemView
{
    Q_OBJECT

public:
    explicit QGraphicsItemViewSymbol(const QPoint &position, QGraphicsScene *scene);
    ~QGraphicsItemViewSymbol();

    enum {Type = QGraphicsItem::UserType + 121};
    int type() const { return Type;}

    void updateView(bool update = false);
    void setViewSymbolFeature(Drawing::FeatureViewSymbol *obj);

    virtual void draw();
    virtual QRectF boundingRect() const;

Q_SIGNALS:
    void hover(bool state);
    void selected(bool state);

protected:
    bool load(QByteArray *svgString);
    void drawSvg();

    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    QGCustomSvg *m_svgItem;
    QSvgRenderer *m_svgRender;
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMVIEWSYMBOL_H

/***************************************************************************
 *   Copyright (c) 2012-2014 Luke Parry <l.parry@warwick.ac.uk>            *
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

#ifndef DRAWINGGUI_QGRAPHICSITEMSVGTEMPLATE_H
#define DRAWINGGUI_QGRAPHICSITEMSVGTEMPLATE_H

#include "QGraphicsItemTemplate.h"

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QGraphicsSvgItem;
class QSvgRenderer;
QT_END_NAMESPACE

namespace Drawing {
class FeatureSVGTemplate;
}

namespace DrawingGui
{

class DrawingGuiExport  QGraphicsItemSVGTemplate : public QGraphicsItemTemplate
{
    Q_OBJECT

public:
    QGraphicsItemSVGTemplate(QGraphicsScene *);
    ~QGraphicsItemSVGTemplate();

    enum {Type = QGraphicsItem::UserType + 153};
    int type() const { return Type;}


    void clearContents();
    void draw();
    virtual void updateView(bool update = false);

Q_SIGNALS:
  void dirty();

protected:
  void openFile(const QFile &file);
  void load (const QString & fileName);

protected:
  Drawing::FeatureSVGTemplate * getSVGTemplate();
  QGraphicsSvgItem *m_svgItem;
  QSvgRenderer *m_svgRender;
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_QGRAPHICSITEMSVGTEMPLATE_H

/***************************************************************************
 *   Copyright (c) 2012-2014 Luke Parry <l.parry@warwick.ac.uk>            *
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

#include "PreCompiled.h"
#ifndef _PreComp_
# include <QAction>
# include <QContextMenuEvent>
# include <QGraphicsScene>
# include <QGraphicsSceneMouseEvent>
# include <QMenu>
# include <QMessageBox>
# include <QMouseEvent>
# include <QPainter>
# include <strstream>
#endif

#include <App/Document.h>
#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Tools2D.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>

#include <Mod/Drawing/App/Geometry.h>
#include <Mod/Drawing/App/FeatureParametricTemplate.h>

#include "QGraphicsItemDrawingTemplate.h"

using namespace DrawingGui;

QGraphicsItemDrawingTemplate::QGraphicsItemDrawingTemplate(QGraphicsScene *scene) : QGraphicsItemTemplate(scene),
                                                                                    pathItem(0)
{
    pathItem = new QGraphicsPathItem;
    pathItem->setParentItem(this);

    // Invert the Y for the QGraphicsPathItem with Y pointing upwards
    QTransform qtrans;
    qtrans.scale(1., -1.);

    this->pathItem->setTransform(qtrans);

    this->addToGroup(pathItem);
}

QGraphicsItemDrawingTemplate::~QGraphicsItemDrawingTemplate()
{
    pathItem = 0;
}

QVariant QGraphicsItemDrawingTemplate::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsItemGroup::itemChange(change, value);
}


void QGraphicsItemDrawingTemplate::clearContents()
{

}

Drawing::FeatureParametricTemplate * QGraphicsItemDrawingTemplate::getParametricTemplate()
{
    if(pageTemplate && pageTemplate->isDerivedFrom(Drawing::FeatureParametricTemplate::getClassTypeId()))
        return static_cast<Drawing::FeatureParametricTemplate *>(pageTemplate);
    else
        return 0;
}

void QGraphicsItemDrawingTemplate::draw()
{

    Drawing::FeatureParametricTemplate *tmplte = getParametricTemplate();
    if(!tmplte)
        throw Base::Exception("Template Feuature not set for QGraphicsItemDrawingTemplate");


    // Clear the previous geometry stored

    // Get a list of geometry and iterate
    const std::vector<DrawingGeometry::BaseGeom *> &geoms =  tmplte->getGeometry();

    std::vector<DrawingGeometry::BaseGeom *>::const_iterator it = geoms.begin();

    QPainterPath path;

    // Draw Edges
    // iterate through all the geometries
    for(; it != geoms.end(); ++it) {
        switch((*it)->geomType) {
          case DrawingGeometry::GENERIC: {

            DrawingGeometry::Generic *geom = static_cast<DrawingGeometry::Generic *>(*it);

            path.moveTo(geom->points[0].fX, geom->points[0].fY);
            std::vector<Base::Vector2D>::const_iterator it = geom->points.begin();

            for(++it; it != geom->points.end(); ++it) {
                path.lineTo((*it).fX, (*it).fY);
            }
          } break;
        }
    }

    this->pathItem->setPath(path);


}

void QGraphicsItemDrawingTemplate::updateView(bool update)
{
    draw();
}

#include "moc_QGraphicsItemDrawingTemplate.cpp"

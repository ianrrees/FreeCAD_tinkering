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

#include "PreCompiled.h"
#ifndef _PreComp_
#include <cmath>
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QGraphicsScene>
#include <QMenu>
#include <QMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainterPathStroker>
#include <QPainter>
#include <QTextOption>
#include <strstream>
#endif

#include <qmath.h>

#include <Base/Console.h>

#include "../App/FeatureViewSection.h"
#include "QGraphicsItemViewSection.h"

using namespace DrawingGui;

QGraphicsItemViewSection::QGraphicsItemViewSection(const QPoint &pos, QGraphicsScene *scene) :QGraphicsItemViewPart(pos, scene)
{
}

QGraphicsItemViewSection::~QGraphicsItemViewSection()
{

}

void QGraphicsItemViewSection::draw()
{
    this->drawSectionFace();
    QGraphicsItemViewPart::draw();
}

void QGraphicsItemViewSection::drawSectionFace()
{
    // Iterate
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewSection::getClassTypeId()))
        return;

    Drawing::FeatureViewSection *part = dynamic_cast<Drawing::FeatureViewSection *>(this->getViewObject());

    Base::Console().Log("drawing section face");

    // Get the section face from the feature
    std::vector<DrawingGeometry::Face *> faceGeoms;
    part->getSectionSurface(faceGeoms);

    // Draw Faces
    std::vector<DrawingGeometry::Face *>::const_iterator fit = faceGeoms.begin();

    QGraphicsItem *graphicsItem = 0;
    QPen facePen;

    for(int i = 0 ; fit != faceGeoms.end(); ++fit, i++) {
        std::vector<DrawingGeometry::Wire *> faceWires = (*fit)->wires;
        QPainterPath facePath;
        for(std::vector<DrawingGeometry::Wire *>::iterator wire = faceWires.begin(); wire != faceWires.end(); ++wire) {
            QPainterPath wirePath;
            QPointF shapePos;
            for(std::vector<DrawingGeometry::BaseGeom *>::iterator baseGeom = (*wire)->geoms.begin(); baseGeom != (*wire)->geoms.end(); ++baseGeom) {
                //Save the start Position
                QPainterPath edgePath = drawPainterPath(*baseGeom);

                // If the current end point matches the shape end point the new edge path needs reversing
                QPointF shapePos = (wirePath.currentPosition()- edgePath.currentPosition());
                if(sqrt(shapePos.x() * shapePos.x() + shapePos.y()*shapePos.y()) < 0.05) {
                    edgePath = edgePath.toReversed();
                }
                wirePath.connectPath(edgePath);
                wirePath.setFillRule(Qt::WindingFill);
            }
            facePath.addPath(wirePath);
        }

        QGraphicsItemFace *item = new QGraphicsItemFace(-1);

        item->setPath(facePath);
        //         item->setStrokeWidth(lineWidth);

        QBrush faceBrush(QBrush(QColor(0,0,255,40)));

        item->setBrush(faceBrush);
        facePen.setColor(Qt::black);
        item->setPen(facePen);
        item->moveBy(this->x(), this->y());
        graphicsItem = dynamic_cast<QGraphicsItem *>(item);

        if(graphicsItem) {
            // Hide any edges that are hidden if option is set.
            //             if((*fit)->extractType == DrawingGeometry::WithHidden && !part->ShowHiddenLines.getValue())
            //                 graphicsItem->hide();

            this->addToGroup(graphicsItem);
            graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }
    }
}

void QGraphicsItemViewSection::updateView(bool update)
{
      // Iterate
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewPart::getClassTypeId()))
        return;

    Drawing::FeatureViewSection *viewPart = dynamic_cast<Drawing::FeatureViewSection *>(this->getViewObject());

    if(update ||
       viewPart->SectionNormal.isTouched() ||
       viewPart->SectionOrigin.isTouched()) {
        QGraphicsItemViewPart::updateView(true);
    } else {
        QGraphicsItemViewPart::updateView();
    }




}

void QGraphicsItemViewSection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsItemViewPart::paint(painter, option, widget);
}

#include "moc_QGraphicsItemViewSection.cpp"

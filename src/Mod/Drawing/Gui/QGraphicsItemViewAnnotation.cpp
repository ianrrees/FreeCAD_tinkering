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

#include "PreCompiled.h"
#ifndef _PreComp_
#include <cmath>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsItem>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsTextItem>
#include <QPainterPathStroker>
#include <QPainter>
#include <QString>
#include <QTextOption>
#include <sstream>
#endif

#include <qmath.h>

#include <App/Application.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

#include "../App/FeatureViewAnnotation.h"
#include "QGraphicsItemViewAnnotation.h"
#include "QGCustomText.h"

using namespace DrawingGui;

QGraphicsItemViewAnnotation::QGraphicsItemViewAnnotation(const QPoint &pos, QGraphicsScene *scene) 
                            :QGraphicsItemView(pos, scene)
{
    setHandlesChildEvents(false);
    setCacheMode(QGraphicsItem::NoCache);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    m_textItem = new QGCustomText();
    this->addToGroup(m_textItem);
    m_textItem->setPos(0.,0.);
}

QGraphicsItemViewAnnotation::~QGraphicsItemViewAnnotation()
{
    // m_textItem belongs to this group and will be deleted by Qt
}

QVariant QGraphicsItemViewAnnotation::itemChange(GraphicsItemChange change, const QVariant &value)
{
    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewAnnotation::setViewAnnoFeature(Drawing::FeatureViewAnnotation *obj)
{
    // called from CanvasView. (once)
    setViewFeature(static_cast<Drawing::FeatureView *>(obj));
}

void QGraphicsItemViewAnnotation::updateView(bool update)
{
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewAnnotation::getClassTypeId()))
        return;

    Drawing::FeatureViewAnnotation *viewAnno = dynamic_cast<Drawing::FeatureViewAnnotation *>(this->getViewObject());

    if (update ||
        viewAnno->isTouched() ||
        viewAnno->Text.isTouched() ||
        viewAnno->Font.isTouched() ||
        viewAnno->TextColor.isTouched() ||
        viewAnno->TextSize.isTouched() ) {

        draw();
    }

    QGraphicsItemView::updateView(update);
}

void QGraphicsItemViewAnnotation::draw()
{
    drawAnnotation();
    if (borderVisible) {
        drawBorder();
    }
}

void QGraphicsItemViewAnnotation::drawAnnotation()
{
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewAnnotation::getClassTypeId()))
        return;

    Drawing::FeatureViewAnnotation *viewAnno = dynamic_cast<Drawing::FeatureViewAnnotation *>(this->getViewObject());

    // get the Text values
    const std::vector<std::string>& annoText = viewAnno->Text.getValues();
    std::stringstream ss;
    for(std::vector<std::string>::const_iterator it = annoText.begin(); it != annoText.end(); it++) {
        if (it == annoText.begin()) {
            ss << *it;
        } else {
            ss << "\n" << *it ;
        }
    }
 
    QFont font;
    font.setFamily(QString::fromUtf8(viewAnno->Font.getValue()));
    font.setPointSizeF(viewAnno->TextSize.getValue()/2.835);           // convert mm <--> points
    m_textItem->setFont(font);

    App::Color c = viewAnno->TextColor.getValue();
    m_textItem->setDefaultTextColor(c.asQColor());

    this->prepareGeometryChange();
    QString qs = QString::fromUtf8(ss.str().c_str());
    m_textItem->setPlainText(qs);
    m_textItem->adjustSize();
    m_textItem->setPos(0.,0.);
}

QRectF QGraphicsItemViewAnnotation::boundingRect() const
{
    return childrenBoundingRect();
}

#include "moc_QGraphicsItemViewAnnotation.cpp"


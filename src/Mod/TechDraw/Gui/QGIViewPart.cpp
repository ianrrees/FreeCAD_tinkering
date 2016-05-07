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

#include "PreCompiled.h"
#ifndef _PreComp_
//    #include <cmath>
    #include <QGraphicsScene>
#endif // #ifndef _PreComp_


#include "App/Application.h"
#include "App/Document.h"
#include "App/DocumentObject.h"
#include "App/Material.h"
#include "Base/Console.h"
#include "Base/Parameter.h"

#include "../App/DrawUtil.h"
#include "../App/DrawViewPart.h"

#include "QGIViewPart.h"

using namespace TechDrawGui;

void _dumpPath(const char* text,QPainterPath path);

const float lineScaleFactor = 1.;   // temp fiddle for devel (also in GIPart.cpp)

QGIViewPart::QGIViewPart()
{
    setHandlesChildEvents(false);
    setCacheMode(QGraphicsItem::NoCache);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, true);

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("HiddenColor", 0x08080800));
    m_colHid = fcColor.asQColor();
}


QGIViewPart::~QGIViewPart()
{
    tidy();
}


QVariant QGIViewPart::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        QList<QGraphicsItem*> items = childItems();
        for(QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); ++it) {
            QGIEdge *edge = dynamic_cast<QGIEdge *>(*it);
            QGIVertex *vert = dynamic_cast<QGIVertex *>(*it);
            if(edge) {
                edge->setHighlighted(isSelected());
            } else if(vert){
                vert->setHighlighted(isSelected());
            }
        }
    } else if(change == ItemSceneChange && scene()) {
           tidy();
    }
    return QGIView::itemChange(change, value);
}


void QGIViewPart::tidy()
{
    //Delete any leftover items
    for(QList<QGraphicsItem*>::iterator it = deleteItems.begin(); it != deleteItems.end(); ++it) {
        delete *it;
    }
    deleteItems.clear();
}


void QGIViewPart::updateView(bool update)
{
    if (getViewObject() == 0 ||
        !getViewObject()->isDerivedFrom(TechDraw::DrawViewPart::getClassTypeId())) {
        return;
    }

    QGIView::updateView(update);

    TechDraw::DrawViewPart *viewPart = dynamic_cast<TechDraw::DrawViewPart *>(getViewObject());

    if(update ||
       viewPart->isTouched() ||
       viewPart->Source.isTouched() ||
       viewPart->Direction.isTouched() ||
       viewPart->Tolerance.isTouched() ||
       viewPart->Scale.isTouched() ||
       viewPart->ShowHiddenLines.isTouched()) {
        // Remove all existing graphical representations (QGIxxxx)  otherwise BRect only grows, never shrinks?
        prepareGeometryChange();
        QList<QGraphicsItem*> items = childItems();
        for(QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); ++it) {
            if (dynamic_cast<QGIEdge *> (*it) ||
                dynamic_cast<QGIFace *>(*it) ||
                dynamic_cast<QGIVertex *>(*it) ||
                dynamic_cast<QGIHatch *>(*it)) {
                removeFromGroup(*it);
                scene()->removeItem(*it);

                // We store these and delete till later to prevent rendering crash ISSUE
                deleteItems.append(*it);
            }
        }
        draw();
    } else if(viewPart->LineWidth.isTouched() ||
              viewPart->HiddenWidth.isTouched()) {
        QList<QGraphicsItem*> items = childItems();
        for(QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); ++it) {
            QGIEdge *edge = dynamic_cast<QGIEdge *>(*it);
            if(edge  && edge->getHiddenEdge()) {
                edge->setStrokeWidth(viewPart->HiddenWidth.getValue() * lineScaleFactor);
            } else {
                edge->setStrokeWidth(viewPart->LineWidth.getValue() * lineScaleFactor);
            }
        }
        draw();
    }
}


void QGIViewPart::draw()
{
    GIPart::draw();
    drawBorder();
}


std::vector<TechDraw::DrawHatch*> QGIViewPart::getHatchesForView(TechDraw::DrawViewPart* viewPart)
{
    std::vector<App::DocumentObject*> docObjs = viewPart->getDocument()->getObjectsOfType(TechDraw::DrawHatch::getClassTypeId());
    std::vector<TechDraw::DrawHatch*> hatchObjs;
    std::string viewName = viewPart->getNameInDocument();
    std::vector<App::DocumentObject*>::iterator itDoc = docObjs.begin();
    for(; itDoc != docObjs.end(); itDoc++) {
        TechDraw::DrawHatch* hatch = dynamic_cast<TechDraw::DrawHatch*>(*itDoc);
        if (viewName.compare((hatch->PartView.getValue())->getNameInDocument()) == 0) {
            hatchObjs.push_back(hatch);
        }
    }
    return hatchObjs;
}


void QGIViewPart::toggleCache(bool state)
{
  QList<QGraphicsItem*> items = childItems();
    for(QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); it++) {
        //(*it)->setCacheMode((state)? DeviceCoordinateCache : NoCache);        //TODO: fiddle cache settings if req'd for performance
        (*it)->setCacheMode((state)? NoCache : NoCache);
        (*it)->update();
    }
}


void QGIViewPart::toggleCosmeticLines(bool state)
{
  QList<QGraphicsItem*> items = childItems();
    for(QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); it++) {
        QGIEdge *edge = dynamic_cast<QGIEdge *>(*it);
        if(edge) {
            edge->setCosmetic(state);
        }
    }
}


void QGIViewPart::toggleVertices(bool state)
{
    QList<QGraphicsItem*> items = childItems();
    for(QList<QGraphicsItem*>::iterator it = items.begin(); it != items.end(); it++) {
        QGIVertex *vert = dynamic_cast<QGIVertex *>(*it);
        if(vert) {
            if(state)
                vert->show();
            else
                vert->hide();
        }
    }
}


QRectF QGIViewPart::boundingRect() const
{
    //return childrenBoundingRect().adjusted(-2.,-2.,2.,6.);             //just a bit bigger than the children need
    return childrenBoundingRect();
}


void _dumpPath(const char* text, QPainterPath path)
{
        QPainterPath::Element elem;
        Base::Console().Message(">>>%s has %d elements\n",text,path.elementCount());
        char* typeName;
        for(int iElem = 0; iElem < path.elementCount(); iElem++) {
            elem = path.elementAt(iElem);
            if(elem.isMoveTo()) {
                typeName = "MoveTo";
            } else if (elem.isLineTo()) {
                typeName = "LineTo";
            } else if (elem.isCurveTo()) {
                typeName = "CurveTo";
            } else {
                typeName = "Unknown";
            }
            Base::Console().Message(">>>>> element %d: type:%d/%s pos(%.3f,%.3f) M:%d L:%d C:%d\n",iElem,
                                    elem.type,typeName,elem.x,elem.y,elem.isMoveTo(),elem.isLineTo(),elem.isCurveTo());
        }
}


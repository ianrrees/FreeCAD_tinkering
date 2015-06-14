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

#include <App/Application.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

#include "../App/FeatureViewPart.h"
#include "QGraphicsItemViewPart.h"

using namespace DrawingGui;

void _dumpPath(const char* text,QPainterPath path);

const float lineScaleFactor = 1.;   // temp fiddle for devel
const float vertexScaleFactor = 2.; // temp fiddle for devel

QGraphicsItemViewPart::QGraphicsItemViewPart(const QPoint &pos, QGraphicsScene *scene)
                :QGraphicsItemView(pos, scene)
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

QGraphicsItemViewPart::~QGraphicsItemViewPart()
{
    tidy();
}

QVariant QGraphicsItemViewPart::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        QList<QGraphicsItem *> items = childItems();
        for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); ++it) {
            QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
            QGraphicsItemVertex *vert = dynamic_cast<QGraphicsItemVertex *>(*it);
            if(edge) {
                edge->setHighlighted(isSelected());
            } else if(vert){
                vert->setHighlighted(isSelected());
            }
        }
    } else if(change == ItemSceneChange && scene()) {
           tidy();
    }
    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewPart::tidy()
{
    //Delete any leftover items
    for(QList<QGraphicsItem *>::iterator it = deleteItems.begin(); it != deleteItems.end(); ++it) {
        delete *it;
    }
    deleteItems.clear();
}

void QGraphicsItemViewPart::setViewPartFeature(Drawing::FeatureViewPart *obj)
{
    // called from CanvasView
    setViewFeature(static_cast<Drawing::FeatureView *>(obj));
}

QPainterPath QGraphicsItemViewPart::drawPainterPath(DrawingGeometry::BaseGeom *baseGeom) const
{
    QPainterPath path;

    switch(baseGeom->geomType) {
        case DrawingGeometry::CIRCLE: {
          DrawingGeometry::Circle *geom = static_cast<DrawingGeometry::Circle *>(baseGeom);

          double x = geom->center.fX - geom->radius;
          double y = geom->center.fY - geom->radius;

          path.addEllipse(x, y, geom->radius * 2, geom->radius * 2);            //topleft@(x,y) radx,rady
          //Base::Console().Message("TRACE -drawPainterPath - making an CIRCLE @(%.3f,%.3f) R:%.3f\n",x, y, geom->radius);

        } break;
        case DrawingGeometry::ARCOFCIRCLE: {
          DrawingGeometry::AOC  *geom = static_cast<DrawingGeometry::AOC *>(baseGeom);

          double startAngle = (geom->startAngle);
          double spanAngle =  (geom->endAngle - startAngle);

          double x = geom->center.fX - geom->radius;
          double y = geom->center.fY - geom->radius;
          pathArc(path, geom->radius, geom->radius, 0., geom->largeArc, geom->cw,
                  geom->endPnt.fX, geom->endPnt.fY,
                  geom->startPnt.fX, geom->startPnt.fY);
          //Base::Console().Message("TRACE -drawPainterPath - making an ARCOFCIRCLE @(%.3f,%.3f) R:%.3f\n",x, y, geom->radius);
        } break;
        case DrawingGeometry::ELLIPSE: {
          DrawingGeometry::Ellipse *geom = static_cast<DrawingGeometry::Ellipse *>(baseGeom);

          double x = geom->center.fX - geom->radius;
          double y = geom->center.fY - geom->radius;

          path.addEllipse(x,y, geom->major * 2, geom->minor * 2);
          //Base::Console().Message("TRACE -drawPainterPath - making an ELLIPSE @(%.3f,%.3f) R1:%.3f R2:%.3f\n",x, y, geom->major, geom->minor);
        } break;
        case DrawingGeometry::ARCOFELLIPSE: {
          DrawingGeometry::AOE *geom = static_cast<DrawingGeometry::AOE *>(baseGeom);

          double startAngle = (geom->startAngle);
          double spanAngle =  (startAngle - geom->endAngle);
          double endAngle = geom->endAngle;

          pathArc(path, geom->major, geom->minor, geom->angle, geom->largeArc, geom->cw,
                        geom->endPnt.fX, geom->endPnt.fY,
                        geom->startPnt.fX, geom->startPnt.fY);
          //Base::Console().Message("TRACE -drawPainterPath - making an ARCOFELLIPSE R1:%.3f R2:%.3f From: (%.3f,%.3f) To: (%.3f,%.3f)\n",geom->major, geom->minor,geom->startPnt.fX, geom->startPnt.fY,geom->endPnt.fX, geom->endPnt.fY);

        } break;
        case DrawingGeometry::BSPLINE: {
          DrawingGeometry::BSpline *geom = static_cast<DrawingGeometry::BSpline *>(baseGeom);

          std::vector<DrawingGeometry::BezierSegment>::const_iterator it = geom->segments.begin();

          // Move painter to the beginning of our first segment
          path.moveTo(it->pnts[0].fX, it->pnts[0].fY);
          //Base::Console().Message("TRACE -drawPainterPath - making an BSPLINE From: (%.3f,%.3f)\n",it->pnts[0].fX,it->pnts[0].fY);

          for ( ; it != geom->segments.end(); ++it) {
              // At this point, the painter is either at the beginning
              // of the first segment, or end of the last
              if ( it->poles == 2 ) {
                  // Degree 1 bezier = straight line...
                  path.lineTo(it->pnts[1].fX, it->pnts[1].fY);

              } else if ( it->poles == 3 ) {
                  path.quadTo(it->pnts[1].fX, it->pnts[1].fY,
                              it->pnts[2].fX, it->pnts[2].fY);

              } else if ( it->poles == 4 ) {
                  path.cubicTo(it->pnts[1].fX, it->pnts[1].fY,
                               it->pnts[2].fX, it->pnts[2].fY,
                               it->pnts[3].fX, it->pnts[3].fY);
              } else {                                                 //can only handle lines,quads,cubes
                  Base::Console().Error("Bad pole count (%d) for BezierSegment of BSpline geometry\n",it->poles);
                  path.lineTo(it->pnts[1].fX, it->pnts[1].fY);         //show something for debugging
              }
          }
        } break;
        case DrawingGeometry::GENERIC: {
          DrawingGeometry::Generic *geom = static_cast<DrawingGeometry::Generic *>(baseGeom);

          path.moveTo(geom->points[0].fX, geom->points[0].fY);
          std::vector<Base::Vector2D>::const_iterator it = geom->points.begin();
          //Base::Console().Message("TRACE -drawPainterPath - making an GENERIC From: (%.3f,%.3f)\n",geom->points[0].fX, geom->points[0].fY); 
          for(++it; it != geom->points.end(); ++it) {
              path.lineTo((*it).fX, (*it).fY);
              //Base::Console().Message(">>>> To: (%.3f,%.3f)\n",(*it).fX, (*it).fY);
          }
        } break;
        default:
          Base::Console().Error("Error - drawPainterPath - UNKNOWN geomType: %d\n",baseGeom->geomType);
          break;
      }

    double rot = getViewObject()->Rotation.getValue();
    if (rot) {
        QTransform t; 
        t.rotate(-rot);
        path = t.map(path);
    }

    return path;
}

void QGraphicsItemViewPart::updateView(bool update)
{
    if (getViewObject() == 0 ||
        !getViewObject()->isDerivedFrom(Drawing::FeatureViewPart::getClassTypeId())) {
        return;
    }

    QGraphicsItemView::updateView(update);

    Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart *>(getViewObject());

    if(update ||
       viewPart->isTouched() ||
       viewPart->Source.isTouched() ||
       viewPart->Direction.isTouched() ||
       viewPart->Tolerance.isTouched() ||
       viewPart->Scale.isTouched() ||
       viewPart->ShowHiddenLines.isTouched()) {
        // Remove all existing graphical representations (QGIxxxx)
        prepareGeometryChange();
        QList<QGraphicsItem *> items = childItems();
        for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); ++it) {
            if (dynamic_cast<QGraphicsItemEdge *> (*it) ||
                dynamic_cast<QGraphicsItemFace *>(*it) ||
                dynamic_cast<QGraphicsItemVertex *>(*it)) {
                removeFromGroup(*it);
                scene()->removeItem(*it);

                // We store these and delete till later to prevent rendering crash ISSUE
                deleteItems.append(*it);
            }
        }
        draw();
    } else if(viewPart->LineWidth.isTouched() ||
              viewPart->HiddenWidth.isTouched()) {
        QList<QGraphicsItem *> items = childItems();
        for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); ++it) {
            QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
            if(edge  && edge->getHiddenEdge()) {
                edge->setStrokeWidth(viewPart->HiddenWidth.getValue() * lineScaleFactor);
            } else {
                edge->setStrokeWidth(viewPart->LineWidth.getValue() * lineScaleFactor);
            }
        }
    }
}

void QGraphicsItemViewPart::draw() {
    drawViewPart();
    drawBorder();
}

void QGraphicsItemViewPart::drawViewPart()
{
    if ( getViewObject() == 0 ||
         !getViewObject()->isDerivedFrom(Drawing::FeatureViewPart::getClassTypeId())) {
        return;
    }

    Drawing::FeatureViewPart *part = dynamic_cast<Drawing::FeatureViewPart *>(getViewObject());

    float lineWidth = part->LineWidth.getValue() * lineScaleFactor;
    float lineWidthHid = part->HiddenWidth.getValue() * lineScaleFactor;
    
    prepareGeometryChange();

#if 0
    // Draw Faces
    const std::vector<DrawingGeometry::Face *> &faceGeoms = part->getFaceGeometry();
    const std::vector<int> &faceRefs = part->getFaceReferences();
    std::vector<DrawingGeometry::Face *>::const_iterator fit = faceGeoms.begin();
    QGraphicsItemFace* face;
    QPen facePen;
    for(int i = 0 ; fit != faceGeoms.end(); fit++, i++) {
        std::vector<DrawingGeometry::Wire *> faceWires = (*fit)->wires;
        QPainterPath facePath;
        for(std::vector<DrawingGeometry::Wire *>::iterator wire = faceWires.begin(); wire != faceWires.end(); wire++) {
            QPainterPath wirePath;
            QPointF shapePos;
            for(std::vector<DrawingGeometry::BaseGeom *>::iterator baseGeom = (*wire)->geoms.begin(); 
                baseGeom != (*wire)->geoms.end();
                baseGeom++) {
                QPainterPath edgePath = drawPainterPath(*baseGeom);
                //If the current end point matches the shape end point the new edge path needs reversing
                QPointF shapePos = (wirePath.currentPosition()- edgePath.currentPosition());
                if(sqrt(shapePos.x() * shapePos.x() + shapePos.y()*shapePos.y()) < 0.05) {
                    edgePath = edgePath.toReversed();
                }
                wirePath.connectPath(edgePath);
                wirePath.setFillRule(Qt::WindingFill);
            }
            facePath.addPath(wirePath);
        }

        //debug a path
        //std::stringstream faceId;
        //faceId << "facePath" << i;
        //_dumpPath(faceId.str().c_str(),facePath);

        QGraphicsItemFace *fitem = new QGraphicsItemFace(-1);
        // TODO: DrawingGeometry::Face has no easy method of determining hidden/visible???
        // Hide any edges that are hidden if option is set.
//      if((*fit)->extractType == DrawingGeometry::WithHidden && !part->ShowHiddenLines.getValue())
//          graphicsItem->hide();
        addToGroup(fitem);
        fitem->setPos(0.0,0.0);
        //QPainterPath simplePath = facePath.simplified();
        //simplePath.setFillRule(Qt::WindingFill);
        //fitem->setPath(simplePath);
        fitem->setPath(facePath);
        fitem->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }
#endif

    // Draw Edges
    const std::vector<DrawingGeometry::BaseGeom *> &geoms = part->getEdgeGeometry();
    const std::vector<int> &refs = part->getEdgeReferences();
    std::vector<DrawingGeometry::BaseGeom *>::const_iterator it = geoms.begin();
    QGraphicsItemEdge* item;

    for(int i = 0 ; it != geoms.end(); ++it, i++) {
        //TODO: investigate if an Edge can be both Hidden and Smooth???
        if(((*it)->extractType == DrawingGeometry::Plain)  ||
          (((*it)->extractType == DrawingGeometry::WithHidden) && part->ShowHiddenLines.getValue()) ||
          ((*it)->extractType == DrawingGeometry::WithSmooth)) {
//          (((*it)->extractType == DrawingGeometry::WithSmooth) && part->ShowSmoothLines.getValue())) {
            item = new QGraphicsItemEdge(refs.at(i));
            addToGroup(item);                                                   //item is at scene(0,0), not group(0,0)
            item->setPos(0.0,0.0);
            item->setStrokeWidth(lineWidth);
            if((*it)->extractType == DrawingGeometry::WithHidden) {
                item->setStrokeWidth(lineWidthHid);
                item->setHiddenEdge(true);
            } else if((*it)->extractType == DrawingGeometry::WithSmooth) {
                item->setSmoothEdge(true);
            }
            item->setPath(drawPainterPath(*it));

            //debug a path
            //QPainterPath edgePath=drawPainterPath(*it);
            //item->setPath(edgePath);
            //std::stringstream edgeId;
            //edgeId << "edge" << i;
            //_dumpPath(edgeId.str().c_str(),edgePath);

            if(refs.at(i) > 0) {
                item->setFlag(QGraphicsItem::ItemIsSelectable, true);  //TODO: bug in App/GeometryObject? why no edge reference?
                item->setAcceptHoverEvents(true);                      //TODO: verify that edge w/o ref is ineligible for selecting
            }
         }
    }

    // Draw Vertexs:
    const std::vector<DrawingGeometry::Vertex *> &verts = part->getVertexGeometry();
    const std::vector<int> &vertRefs                    = part->getVertexReferences();
    std::vector<DrawingGeometry::Vertex *>::const_iterator vert = verts.begin();

    for(int i = 0 ; vert != verts.end(); ++vert, i++) {
        QGraphicsItemVertex *item = new QGraphicsItemVertex(vertRefs.at(i));
        addToGroup(item);
        item->setPos((*vert)->pnt.fX, (*vert)->pnt.fY);                //this is in ViewPart coords
        item->setRadius(lineWidth * vertexScaleFactor);
        if(vertRefs.at(i) > 0) {
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            item->setAcceptHoverEvents(true);                      //TODO: verify that vertex w/o ref is ineligible for selecting
        }
     }
}

void QGraphicsItemViewPart::pathArc(QPainterPath &path, double rx, double ry, double x_axis_rotation,
                                    bool large_arc_flag, bool sweep_flag,
                                    double x, double y,
                                    double curx, double cury) const
{
    double sin_th, cos_th;
    double a00, a01, a10, a11;
    double x0, y0, x1, y1, xc, yc;
    double d, sfactor, sfactor_sq;
    double th0, th1, th_arc;
    int i, n_segs;
    double dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;

    rx = qAbs(rx);
    ry = qAbs(ry);

    sin_th = qSin(x_axis_rotation * (M_PI / 180.0));
    cos_th = qCos(x_axis_rotation * (M_PI / 180.0));

    dx = (curx - x) / 2.0;
    dy = (cury - y) / 2.0;
    dx1 =  cos_th * dx + sin_th * dy;
    dy1 = -sin_th * dx + cos_th * dy;
    Pr1 = rx * rx;
    Pr2 = ry * ry;
    Px = dx1 * dx1;
    Py = dy1 * dy1;
    /* Spec : check if radii are large enough */
    check = Px / Pr1 + Py / Pr2;
    if (check > 1) {
        rx = rx * qSqrt(check);
        ry = ry * qSqrt(check);
    }

    a00 =  cos_th / rx;
    a01 =  sin_th / rx;
    a10 = -sin_th / ry;
    a11 =  cos_th / ry;
    x0 = a00 * curx + a01 * cury;
    y0 = a10 * curx + a11 * cury;
    x1 = a00 * x + a01 * y;
    y1 = a10 * x + a11 * y;
    /* (x0, y0) is current point in transformed coordinate space.
       (x1, y1) is new point in transformed coordinate space.

       The arc fits a unit-radius circle in this space.
    */
    d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
    sfactor_sq = 1.0 / d - 0.25;
    if (sfactor_sq < 0)
        sfactor_sq = 0;

    sfactor = qSqrt(sfactor_sq);

    if (sweep_flag == large_arc_flag)
        sfactor = -sfactor;

    xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
    yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
    /* (xc, yc) is center of the circle. */

    th0 = qAtan2(y0 - yc, x0 - xc);
    th1 = qAtan2(y1 - yc, x1 - xc);

    th_arc = th1 - th0;
    if (th_arc < 0 && sweep_flag)
        th_arc += 2 * M_PI;
    else if (th_arc > 0 && !sweep_flag)
        th_arc -= 2 * M_PI;

    n_segs = qCeil(qAbs(th_arc / (M_PI * 0.5 + 0.001)));

    path.moveTo(curx, cury);

    for (i = 0; i < n_segs; i++) {
        pathArcSegment(path, xc, yc,
                       th0 + i * th_arc / n_segs,
                       th0 + (i + 1) * th_arc / n_segs,
                       rx, ry, x_axis_rotation);
    }
}

void QGraphicsItemViewPart::pathArcSegment(QPainterPath &path,
                                           double xc, double yc,
                                           double th0, double th1,
                                           double rx, double ry, double xAxisRotation) const
{
    double sinTh, cosTh;
    double a00, a01, a10, a11;
    double x1, y1, x2, y2, x3, y3;
    double t;
    double thHalf;

    sinTh = qSin(xAxisRotation * (M_PI / 180.0));
    cosTh = qCos(xAxisRotation * (M_PI / 180.0));

    a00 =  cosTh * rx;
    a01 = -sinTh * ry;
    a10 =  sinTh * rx;
    a11 =  cosTh * ry;

    thHalf = 0.5 * (th1 - th0);
    t = (8.0 / 3.0) * qSin(thHalf * 0.5) * qSin(thHalf * 0.5) / qSin(thHalf);
    x1 = xc + qCos(th0) - t * qSin(th0);
    y1 = yc + qSin(th0) + t * qCos(th0);
    x3 = xc + qCos(th1);
    y3 = yc + qSin(th1);
    x2 = x3 + t * qSin(th1);
    y2 = y3 - t * qCos(th1);

    path.cubicTo(a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
                 a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
                 a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

QGraphicsItemEdge * QGraphicsItemViewPart::findRefEdge(int idx)
{
    QList<QGraphicsItem *> items = childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
        if(edge && edge->getReference() == idx)
            return edge;
    }
    return 0;
}

QGraphicsItemVertex * QGraphicsItemViewPart::findRefVertex(int idx)
{
    QList<QGraphicsItem *> items = childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        QGraphicsItemVertex *vert = dynamic_cast<QGraphicsItemVertex *>(*it);
        if(vert && vert->getReference() == idx)
            return vert;
    }
    return 0;
}

void QGraphicsItemViewPart::toggleCache(bool state)
{
  QList<QGraphicsItem *> items = childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        //(*it)->setCacheMode((state)? DeviceCoordinateCache : NoCache);        //TODO: fiddle cache settings if req'd for performance
        (*it)->setCacheMode((state)? NoCache : NoCache);
        (*it)->update();
    }
}

void QGraphicsItemViewPart::toggleCosmeticLines(bool state)
{
  QList<QGraphicsItem *> items = childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
        if(edge) {
            edge->setCosmetic(state);
        }
    }
}

void QGraphicsItemViewPart::toggleVertices(bool state)
{
    QList<QGraphicsItem *> items = childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        QGraphicsItemVertex *vert = dynamic_cast<QGraphicsItemVertex *>(*it);
        if(vert) {
            if(state)
                vert->show();
            else
                vert->hide();
        }
    }
}

QRectF QGraphicsItemViewPart::boundingRect() const
{
    //return childrenBoundingRect().adjusted(-2.,-2.,2.,6.);             //just a bit bigger than the children need
    return childrenBoundingRect();
}

void _dumpPath(const char* text,QPainterPath path)
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


#include "moc_QGraphicsItemViewPart.cpp"


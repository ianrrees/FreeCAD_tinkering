/***************************************************************************
 *   Copyright (c) 2016                    Ian Rees <ian.rees@gmail.com>   *
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
    #include <QGraphicsItem>
    #include <qmath.h>  // TODO: Break dependency on this
#endif // #ifndef _PreComp_

#include "Base/Console.h"

#include "GIEdge.h"
#include "GIFace.h"
#include "GIHatch.h"
#include "GIVertex.h"
#include "ZVALUE.h"

#include "../DrawHatch.h"
#include "../DrawUtil.h"

#include "GIPart.h"

using namespace TechDraw;
using TechDrawGeometry::ecHARD;
using TechDrawGeometry::ecOUTLINE;
using TechDrawGeometry::ecSMOOTH;
using TechDrawGeometry::ecSEAM;

const float lineScaleFactor = 1.;   // temp fiddle for devel (also in QGIViewPart.cpp)
const float vertexScaleFactor = 2.; // temp fiddle for devel

void GIPart::draw()
{
    if ( getViewObject() == 0 ||
         !getViewObject()->isDerivedFrom(TechDraw::DrawViewPart::getClassTypeId())) {
        return;
    }

    TechDraw::DrawViewPart *viewPart = dynamic_cast<TechDraw::DrawViewPart *>(getViewObject());

    float lineWidth = viewPart->LineWidth.getValue() * lineScaleFactor;
    float lineWidthHid = viewPart->HiddenWidth.getValue() * lineScaleFactor;

    prepareGeometryChange();

#if MOD_TECHDRAW_HANDLE_FACES
    // Draw Faces
    const std::vector<TechDrawGeometry::Face *> &faceGeoms = viewPart->getFaceGeometry();
    std::vector<TechDrawGeometry::Face *>::const_iterator fit = faceGeoms.begin();
    QPen facePen;
    facePen.setCosmetic(true);
    //QBrush faceBrush;
    for(int i = 0 ; fit != faceGeoms.end(); fit++, i++) {
        GIFace *newFace = drawFace(*fit);
        newFace->setPen(facePen);
        newFace->setZValue(ZVALUE::FACE);
        //newFace->setBrush(faceBrush);
    }
    //debug a path
    //std::stringstream faceId;
    //faceId << "facePath" << i;
    //_dumpPath(faceId.str().c_str(),facePath);
#endif //#if MOD_TECHDRAW_HANDLE_FACES

    // Draw Hatches
    std::vector<TechDraw::DrawHatch*> hatchObjs = viewPart->getHatches();
    if (!hatchObjs.empty()) {
        std::vector<TechDraw::DrawHatch*>::iterator itHatch = hatchObjs.begin();
        for(; itHatch != hatchObjs.end(); itHatch++) {
            //if hatchdirection == viewPartdirection {
            auto feat(*itHatch);
            const std::vector<std::string> &edgeNames = feat->Edges.getSubValues();
            std::vector<std::string>::const_iterator itEdge = edgeNames.begin();
            std::vector<TechDrawGeometry::BaseGeom*> unChained;

            //get all edge geometries for this hatch
            for (; itEdge != edgeNames.end(); itEdge++) {
                int idxEdge = TechDraw::DrawUtil::getIndexFromName((*itEdge));
                TechDrawGeometry::BaseGeom* edgeGeom = viewPart->getProjEdgeByIndex(idxEdge);
                if (!edgeGeom) {
                    Base::Console().Log("Error - qgivp::drawViewPart - edgeGeom: %d is NULL\n",idxEdge);
                }
                unChained.push_back(edgeGeom);
            }

            //chain edges tail to nose into a closed region
            auto chained( TechDrawGeometry::GeometryUtils::chainGeoms(unChained) );

            //iterate through the chain to make QPainterPath
            std::vector<TechDrawGeometry::BaseGeom*>::iterator itChain = chained.begin();
            QPainterPath hatchPath;
            for (; itChain != chained.end(); itChain++) {
                QPainterPath subPath;
                if ((*itChain)->reversed) {
                    subPath = drawPainterPath((*itChain)).toReversed();
                } else {
                    subPath = drawPainterPath((*itChain));
                }
                hatchPath.connectPath(subPath);
                //_dumpPath("subpath",subPath);
            }

            auto hatch( new GIHatch(feat->getNameInDocument()) );
            addToGroup(hatch);
            hatch->setPos(0.0,0.0);
            hatch->setPath(hatchPath);
            hatch->setFill(feat->HatchPattern.getValue());
            hatch->setColor(feat->HatchColor.getValue());
            //_dumpPath("hatchPath",hatchPath);
            hatch->setFlag(QGraphicsItem::ItemIsSelectable, true);
            hatch->setZValue(ZVALUE::HATCH);
        }
    }

    // Draw Edges
    const std::vector<TechDrawGeometry::BaseGeom *> &geoms = viewPart->getEdgeGeometry();
    std::vector<TechDrawGeometry::BaseGeom *>::const_iterator itEdge = geoms.begin();

    for(int i = 0 ; itEdge != geoms.end(); itEdge++, i++) {
        bool showEdge = false;
        if ((*itEdge)->visible) {
            if (((*itEdge)->classOfEdge == ecHARD) ||
                ((*itEdge)->classOfEdge == ecOUTLINE) ||
                (((*itEdge)->classOfEdge == ecSMOOTH) && viewPart->ShowSmoothLines.getValue()) ||
                (((*itEdge)->classOfEdge == ecSEAM) && viewPart->ShowSeamLines.getValue())) {
                showEdge = true;
            }
        } else {
            if (viewPart->ShowHiddenLines.getValue()) {
                showEdge = true;
            }
        }
        if (showEdge) {
            auto item( new GIEdge(i) );
            addToGroup(item);                                                   //item is at scene(0,0), not group(0,0)
            item->setPos(0.0,0.0);
            item->setPath(drawPainterPath(*itEdge));
            item->setStrokeWidth(lineWidth);
            item->setZValue(ZVALUE::EDGE);
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
            item->setAcceptHoverEvents(true);
            if(!(*itEdge)->visible) {
                item->setStrokeWidth(lineWidthHid);
                item->setHiddenEdge(true);
                item->setZValue(ZVALUE::HIDEDGE);
            }
//TODO - necessary?            item->setPrettyNormal();
            //debug a path
            //QPainterPath edgePath=drawPainterPath(*itEdge);
            //std::stringstream edgeId;
            //edgeId << "QGIVP.edgePath" << i;
            //_dumpPath(edgeId.str().c_str(),edgePath);
         }
    }

    // Draw Vertexs:
    const std::vector<TechDrawGeometry::Vertex *> &verts = viewPart->getVertexGeometry();
    std::vector<TechDrawGeometry::Vertex *>::const_iterator vert = verts.begin();
    for(int i = 0 ; vert != verts.end(); ++vert, i++) {
        auto item( new GIVertex(i) );
        addToGroup(item);
        item->setPos((*vert)->pnt.fX, (*vert)->pnt.fY);                //this is in ViewPart coords
        item->setRadius(lineWidth * vertexScaleFactor);
        item->setZValue(ZVALUE::VERTEX);
     }
}


GIFace * GIPart::drawFace(TechDrawGeometry::Face* f)
{
    std::vector<TechDrawGeometry::Wire *> fWires = f->wires;
    QPainterPath facePath;
    for(std::vector<TechDrawGeometry::Wire *>::iterator wire = fWires.begin(); wire != fWires.end(); ++wire) {
        QPainterPath wirePath;
        for(std::vector<TechDrawGeometry::BaseGeom *>::iterator edge = (*wire)->geoms.begin(); edge != (*wire)->geoms.end(); ++edge) {
            //Save the start Position
            QPainterPath edgePath = drawPainterPath(*edge);
            // If the current end point matches the shape end point the new edge path needs reversing
            QPointF shapePos = (wirePath.currentPosition()- edgePath.currentPosition());
            if(sqrt(shapePos.x() * shapePos.x() + shapePos.y()*shapePos.y()) < 0.05) {    //magic tolerance
                edgePath = edgePath.toReversed();
            }
            wirePath.connectPath(edgePath);
            wirePath.setFillRule(Qt::WindingFill);
        }
        facePath.addPath(wirePath);
    }

    auto gFace( new GIFace(-1) );
    addToGroup(gFace);
    gFace->setPos(0.0,0.0);
    gFace->setPath(facePath);
    //_dumpPath("QGIVP.facePath",facePath);

    //gFace->setFlag(QGraphicsItem::ItemIsSelectable, true);   ???
    return gFace;
}

QPainterPath GIPart::drawPainterPath(TechDrawGeometry::BaseGeom *baseGeom) const
{
    QPainterPath path;

    switch(baseGeom->geomType) {
        case TechDrawGeometry::CIRCLE: {
          TechDrawGeometry::Circle *geom = static_cast<TechDrawGeometry::Circle *>(baseGeom);

          double x = geom->center.fX - geom->radius;
          double y = geom->center.fY - geom->radius;

          path.addEllipse(x, y, geom->radius * 2, geom->radius * 2);            //topleft@(x,y) radx,rady
          //Base::Console().Message("TRACE -drawPainterPath - making an CIRCLE @(%.3f,%.3f) R:%.3f\n",x, y, geom->radius);

        } break;
        case TechDrawGeometry::ARCOFCIRCLE: {
          TechDrawGeometry::AOC  *geom = static_cast<TechDrawGeometry::AOC *>(baseGeom);

          //double x = geom->center.fX - geom->radius;
          //double y = geom->center.fY - geom->radius;
          pathArc(path, geom->radius, geom->radius, 0., geom->largeArc, geom->cw,
                  geom->endPnt.fX, geom->endPnt.fY,
                  geom->startPnt.fX, geom->startPnt.fY);
          //Base::Console().Message("TRACE -drawPainterPath - making an ARCOFCIRCLE @(%.3f,%.3f) R:%.3f\n",x, y, geom->radius);
        } break;
        case TechDrawGeometry::ELLIPSE: {
          TechDrawGeometry::Ellipse *geom = static_cast<TechDrawGeometry::Ellipse *>(baseGeom);

          // Calculate start and end points as ellipse with theta = 0 and pi
          double startX = geom->center.fX + geom->major * cos(geom->angle),
                 startY = geom->center.fY + geom->major * sin(geom->angle),
                 endX = geom->center.fX - geom->major * cos(geom->angle),
                 endY = geom->center.fY - geom->major * sin(geom->angle);

          pathArc(path, geom->major, geom->minor, geom->angle, false, false,
                  endX, endY, startX, startY);

          pathArc(path, geom->major, geom->minor, geom->angle, false, false,
                  startX, startY, endX, endY);

          //Base::Console().Message("TRACE -drawPainterPath - making an ELLIPSE @(%.3f,%.3f) R1:%.3f R2:%.3f\n",x, y, geom->major, geom->minor);
        } break;
        case TechDrawGeometry::ARCOFELLIPSE: {
          TechDrawGeometry::AOE *geom = static_cast<TechDrawGeometry::AOE *>(baseGeom);

          pathArc(path, geom->major, geom->minor, geom->angle, geom->largeArc, geom->cw,
                        geom->endPnt.fX, geom->endPnt.fY,
                        geom->startPnt.fX, geom->startPnt.fY);
          //Base::Console().Message("TRACE -drawPainterPath - making an ARCOFELLIPSE R1:%.3f R2:%.3f From: (%.3f,%.3f) To: (%.3f,%.3f)\n",geom->major, geom->minor,geom->startPnt.fX, geom->startPnt.fY,geom->endPnt.fX, geom->endPnt.fY);

        } break;
        case TechDrawGeometry::BSPLINE: {
          TechDrawGeometry::BSpline *geom = static_cast<TechDrawGeometry::BSpline *>(baseGeom);

          std::vector<TechDrawGeometry::BezierSegment>::const_iterator it = geom->segments.begin();

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
        case TechDrawGeometry::GENERIC: {
          TechDrawGeometry::Generic *geom = static_cast<TechDrawGeometry::Generic *>(baseGeom);

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


void GIPart::pathArcSegment( QPainterPath &path,
                             double xc, double yc,
                             double th0, double th1,
                             double rx, double ry, double xAxisRotation ) const
{
    double sinTh, cosTh;
    double a00, a01, a10, a11;
    double x1, y1, x2, y2, x3, y3;
    double t;
    double thHalf;

    sinTh = qSin(xAxisRotation);
    cosTh = qCos(xAxisRotation);

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


// As called by arc of ellipse case:
// pathArc(path, geom->major, geom->minor, geom->angle, geom->largeArc, geom->cw,
//         geom->endPnt.fX, geom->endPnt.fY,
//         geom->startPnt.fX, geom->startPnt.fY);
void GIPart::pathArc( QPainterPath &path, double rx, double ry, double x_axis_rotation,
                      bool large_arc_flag, bool sweep_flag,
                      double x, double y,
                      double curx, double cury ) const
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

    sin_th = qSin(x_axis_rotation);
    cos_th = qCos(x_axis_rotation);

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


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

const float lineScaleFactor = 3.;

QGraphicsItemViewPart::QGraphicsItemViewPart(const QPoint &pos, QGraphicsScene *scene)
                :QGraphicsItemView(pos, scene),
                 borderVisible(true)
{
    setHandlesChildEvents(false);
    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("HiddenColor", 0x08080800));
    m_colHid = fcColor.asQColor();
    pen.setColor(m_colNormal);

    //setCacheMode(QGraphicsItem::NoCache);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
}

QGraphicsItemViewPart::~QGraphicsItemViewPart()
{
    // TODO: Identify what changed to prevent complete redraw
    //QList<QGraphicsItem *> items = this->childItems();
    //QList<QGraphicsItem *> bboxItems = items;
    //bbox.setSize(QSizeF(0.,0.));
}

QVariant QGraphicsItemViewPart::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        QColor color;
        if(isSelected()) {
            pen.setColor(m_colSel);

        } else {
            pen.setColor(m_colNormal);
        }

        QList<QGraphicsItem *> items = this->childItems();
        for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); ++it) {

            QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
            QGraphicsItemVertex *vert = dynamic_cast<QGraphicsItemVertex *>(*it);
            if(edge) {
                edge->setHighlighted(isSelected());
            } else if(vert){
                QBrush brush = vert->brush();
                brush.setColor(color);
                vert->setBrush(brush);
            }
        }
        update();
    } else if(change == ItemSceneChange && scene()) {
        // if item has been removed from scene we need to update bounding box to not draw anything

            // NOTE:  Temporary solution to prevent segfaulting in PaintDraw event
           borderVisible = false;
           this->tidy();
//         prepareGeometryChange();
//         bbox.setHeight(0.);
//         bbox.setWidth(0.);
    }
    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewPart::tidy()
{
    //Delete any leftover items
    for(QList<QGraphicsItem *>::iterator it = deleteItems.begin(); it != deleteItems.end(); ++it) {
          delete *it;
    }
}

void QGraphicsItemViewPart::setViewPartFeature(Drawing::FeatureViewPart *obj)
{
    if(obj == 0)
        return;

    this->setViewFeature(static_cast<Drawing::FeatureView *>(obj));
    this->draw();

    // Set the QGraphicsItemGroup Properties based on the FeatureView
    float x = obj->X.getValue();
    float y = obj->Y.getValue();

    this->setPos(x, y);
    Q_EMIT dirty();
}

QPainterPath QGraphicsItemViewPart::drawPainterPath(DrawingGeometry::BaseGeom *baseGeom) const
{
    QPainterPath path;
    switch(baseGeom->geomType) {
        case DrawingGeometry::CIRCLE: {
          DrawingGeometry::Circle *geom = static_cast<DrawingGeometry::Circle *>(baseGeom);

          double x = geom->center.fX - geom->radius;
          double y = geom->center.fY - geom->radius;

          path.addEllipse(x, y, geom->radius * 2, geom->radius * 2);

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
        } break;
        case DrawingGeometry::ELLIPSE: {
          DrawingGeometry::Ellipse *geom = static_cast<DrawingGeometry::Ellipse *>(baseGeom);

          double x = geom->center.fX - geom->radius;
          double y = geom->center.fY - geom->radius;

          path.addEllipse(x,y, geom->major * 2, geom->minor * 2);
        } break;
        case DrawingGeometry::ARCOFELLIPSE: {
          DrawingGeometry::AOE *geom = static_cast<DrawingGeometry::AOE *>(baseGeom);

          double startAngle = (geom->startAngle);
          double spanAngle =  (startAngle - geom->endAngle);
          double endAngle = geom->endAngle;

          this->pathArc(path, geom->major, geom->minor, geom->angle, geom->largeArc, geom->cw,
                        geom->endPnt.fX, geom->endPnt.fY,
                        geom->startPnt.fX, geom->startPnt.fY);

        } break;
        case DrawingGeometry::BSPLINE: {
          DrawingGeometry::BSpline *geom = static_cast<DrawingGeometry::BSpline *>(baseGeom);

          std::vector<DrawingGeometry::BezierSegment>::const_iterator it = geom->segments.begin();

          DrawingGeometry::BezierSegment startSeg = geom->segments.at(0);
          path.moveTo(startSeg.pnts[0].fX, startSeg.pnts[0].fY);
          Base::Vector2D prevContPnt = startSeg.pnts[1];

          for(int i = 0; it != geom->segments.end(); ++it, ++i) {
              DrawingGeometry::BezierSegment seg = *it;
              if(seg.poles == 4) {
                  path.cubicTo(seg.pnts[1].fX,seg.pnts[1].fY, seg.pnts[2].fX, seg.pnts[2].fY, seg.pnts[3].fX, seg.pnts[3].fY);
              } else {
                  Base::Vector2D cPnt;
                  if(i == 0) {
                      prevContPnt.Set(startSeg.pnts[1].fX, startSeg.pnts[1].fX);
                  } else {
                      prevContPnt.Set(2 * startSeg.pnts[1].fX - prevContPnt.fX, 2 * startSeg.pnts[1].fY - prevContPnt.fY);
                  }

                  path.quadTo(prevContPnt.fX, prevContPnt.fY, seg.pnts[2].fX, seg.pnts[2].fY);

              }
          }
        } break;
        case DrawingGeometry::GENERIC: {
          DrawingGeometry::Generic *geom = static_cast<DrawingGeometry::Generic *>(baseGeom);

          path.moveTo(geom->points[0].fX, geom->points[0].fY);
          std::vector<Base::Vector2D>::const_iterator it = geom->points.begin();

          for(++it; it != geom->points.end(); ++it) {
              path.lineTo((*it).fX, (*it).fY);
          }
        } break;
        default:
          break;
      }
      return path;
}
void QGraphicsItemViewPart::updateView(bool update)
{
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewPart::getClassTypeId()))
        return;

    Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart *>(this->getViewObject());

    if(update ||
       viewPart->isTouched() ||
       viewPart->Source.isTouched() ||
       viewPart->Direction.isTouched() ||
       viewPart->Tolerance.isTouched() ||
       viewPart->Scale.isTouched() ||
       viewPart->ShowHiddenLines.isTouched()){

        Base::Console().Log("Drawing::QGraphicsItemViewPart::updateView - Shapes need redrawing %s\n", viewPart->getNameInDocument());

        // Identify what changed to prevent complete redraw
        QList<QGraphicsItem *> items = this->childItems();
        QList<QGraphicsItem *> bboxItems = items;

        bbox.setSize(QSizeF(0,0));
        for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); ++it) {
            // Rebuild bounding box on every deletion - unfortunatly necessary
            QRectF tmpBox;
            for(QList<QGraphicsItem *>::iterator qit = bboxItems.begin(); qit!= bboxItems.end(); ++qit) {
                if(*qit)
                    tmpBox |=  this->transform().mapRect((*qit)->boundingRect());
            }

            // Declare the bounding box will change and set to new one without element
            prepareGeometryChange();
            bbox = tmpBox;

            if(dynamic_cast<QGraphicsItemEdge *> (*it) ||
              dynamic_cast<QGraphicsItemFace *>(*it) ||
              dynamic_cast<QGraphicsItemVertex *>(*it)) {
                this->scene()->removeItem(*it);
                deleteItems.append(*it); // We store these and delete till later to prevent rendering crash ISSUE
            }
            bboxItems.removeFirst();
        }

        // Redraw the part
        draw();
    } else if(viewPart->LineWidth.isTouched()) {
        Base::Console().Log("Drawing::QGraphicsItemViewPart::updateView - line width touched \n");
        QList<QGraphicsItem *> items = this->childItems();
        for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); ++it) {
            QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);

            float lineWidth = viewPart->LineWidth.getValue() * lineScaleFactor;
            if(edge)
                edge->setStrokeWidth(lineWidth);
        }
    }

    QGraphicsItemView::updateView(update);

}

void QGraphicsItemViewPart::draw() {
    this->drawViewPart();
}

void QGraphicsItemViewPart::drawViewPart()
{
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewPart::getClassTypeId()))
        return;

    Drawing::FeatureViewPart *part = dynamic_cast<Drawing::FeatureViewPart *>(this->getViewObject());

    float lineWidth = part->LineWidth.getValue() * lineScaleFactor;

    QRectF box;

    // Draw Faces
    QGraphicsItem *graphicsItem = 0;
    const std::vector<DrawingGeometry::Face *> &faceGeoms = part->getFaceGeometry();
    const std::vector<int> &faceRefs = part->getFaceReferences();
    std::vector<DrawingGeometry::Face *>::const_iterator fit = faceGeoms.begin();

    QPen facePen;
    for(int i = 0 ; fit != faceGeoms.end(); ++fit, i++) {
        std::vector<DrawingGeometry::Wire *> faceWires = (*fit)->wires;
        QPainterPath facePath;
        for(std::vector<DrawingGeometry::Wire *>::iterator wire = faceWires.begin(); wire != faceWires.end(); ++wire) {
            QPainterPath wirePath;
            QPointF shapePos;
            for(std::vector<DrawingGeometry::BaseGeom *>::iterator baseGeom = (*wire)->geoms.begin(); 
                baseGeom != (*wire)->geoms.end();
                ++baseGeom) {
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

        item->moveBy(this->x(), this->y());
        QPointF posRef(0.,0.);
        QPointF mapPos = item->mapToItem(this, posRef);
        item->moveBy(-mapPos.x(), -mapPos.y());
        graphicsItem = dynamic_cast<QGraphicsItem *>(item);
        if(graphicsItem) {
            // TODO: DrawingGeometry::Face has no easy method of determining hidden/visible!!!
            //Base::Console().Message("DEBUG - QGraphicsItemViewPart::drawViewPart - face: %d extract: %d show: %d\n",
            //    i,(*fit)->extractType,part->ShowHiddenLines.getValue());
            // Hide any edges that are hidden if option is set.
//             if((*fit)->extractType == DrawingGeometry::WithHidden && !part->ShowHiddenLines.getValue())
//                 graphicsItem->hide();
            this->addToGroup(graphicsItem);
            graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }
    } 


    // Draw Edges
    graphicsItem = 0;
    const std::vector<DrawingGeometry::BaseGeom *> &geoms = part->getEdgeGeometry();
    const std::vector<int> &refs = part->getEdgeReferences();
    std::vector<DrawingGeometry::BaseGeom *>::const_iterator it = geoms.begin();

    // Draw Edges
    // iterate through all the geometries
    for(int i = 0 ; it != geoms.end(); ++it, i++) {
       // Attempt to find if a previous edge exists
      QGraphicsItemEdge *item = this->findRefEdge(refs.at(i));

      if(!item) {
          item = new QGraphicsItemEdge(refs.at(i));

          // Edges and Vertexs must be transformed to the ViewPart's coordinate system
          item->moveBy(this->x(), this->y());
          QPointF posRef(0.,0.);
          QPointF mapPos = item->mapToItem(this, posRef);
          item->moveBy(-mapPos.x(), -mapPos.y());
          if(part->ShowHiddenLines.getValue()) {
              item->setShowHidden(true);
        }
      }
      item->setStrokeWidth(lineWidth);

      graphicsItem = dynamic_cast<QGraphicsItem *>(item);
      QPainterPath path = drawPainterPath(*it);
      if(graphicsItem) {
          if((*it)->extractType == DrawingGeometry::WithHidden) {
              QPainterPath hPath  = item->getHiddenPath();
              hPath.addPath(path);
              item->setHiddenPath(hPath);
          } else {
              QPainterPath vPath  = item->getVisiblePath();
              vPath.addPath(path);
              item->setVisiblePath(vPath);
          }

          box |=  this->transform().mapRect(graphicsItem->boundingRect());
          prepareGeometryChange();

          bbox = box;

          this->addToGroup(graphicsItem);
          // Don't allow selection for any edges with no references
          if(refs.at(i) > 0) {
              graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
          }
      }
    }

    graphicsItem = 0;

    // Draw Vertexs:
    const std::vector<DrawingGeometry::Vertex *> &verts = part->getVertexGeometry();
    const std::vector<int> &vertRefs                    = part->getVertexReferences();

    std::vector<DrawingGeometry::Vertex *>::const_iterator vert = verts.begin();

    QBrush vertBrush(QBrush(QColor(0,0,0,255)));

     // iterate through all the geometries
    for(int i = 0 ; vert != verts.end(); ++vert, i++) {
          DrawingGeometry::Vertex *myVertex = *vert;
          QGraphicsItemVertex *item = new QGraphicsItemVertex(vertRefs.at(i));
          QPainterPath path;
          //item->setBrush(vertBrush);
          path.addEllipse(-3 ,-3, 6, 6);     //TODO: these should change size with the view, or when viewpart is selected?

          QPointF posRef(0.,0.);
          QPointF mapPos = item->mapToItem(this, posRef);

          item->setPath(path);
          item->setPos(myVertex->pnt.fX, myVertex->pnt.fY);
          item->moveBy(-mapPos.x(), -mapPos.y());
          if(vertRefs.at(i) > 0)
              item->setFlag(QGraphicsItem::ItemIsSelectable, true);

          // Edges and Vertexs must be transformed to the ViewPart's coordinate system
          //item->moveBy(this->x(), this->y());
          this->addToGroup(item);
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
    QList<QGraphicsItem *> items = this->childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
        if(edge && edge->getReference() == idx)
            return edge;
    }
    return 0;
}

QGraphicsItemVertex * QGraphicsItemViewPart::findRefVertex(int idx)
{
    QList<QGraphicsItem *> items = this->childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        QGraphicsItemVertex *vert = dynamic_cast<QGraphicsItemVertex *>(*it);
        if(vert && vert->getReference() == idx)
            return vert;
    }
    return 0;
}

void QGraphicsItemViewPart::toggleCache(bool state)
{
  QList<QGraphicsItem *> items = this->childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        //(*it)->setCacheMode((state)? DeviceCoordinateCache : NoCache);
        (*it)->setCacheMode((state)? NoCache : NoCache);
        (*it)->update();
    }
}


void QGraphicsItemViewPart::toggleCosmeticLines(bool state)
{
  QList<QGraphicsItem *> items = this->childItems();
    for(QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); it++) {
        QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
        if(edge) {
            edge->setCosmetic(state);
        }
    }
}

void QGraphicsItemViewPart::toggleVertices(bool state)
{
    QList<QGraphicsItem *> items = this->childItems();
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

void QGraphicsItemViewPart::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // TODO don't like this but only solution at the minute
    if(this->shape().contains(event->pos())) {
        pen.setColor(m_colPre);
    }
    update();
}

void QGraphicsItemViewPart::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(!isSelected()) {
        pen.setColor(m_colNormal);
        update();
    }
}

QPainterPath QGraphicsItemViewPart::shape() const {
    QPainterPath path;
    QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);
    path.addRect(box);
    QPainterPathStroker stroker;
    stroker.setWidth(5.f);
    return stroker.createStroke(path);
}

QRectF QGraphicsItemViewPart::boundingRect() const
{
    return QGraphicsItemView::boundingRect().adjusted(-5.,-5.,5.,5.);
}

void QGraphicsItemViewPart::drawBorder(QPainter *painter)
{
  // Save the current painter state and restore at end
  painter->save();

  // Make a rectangle smaller than the bounding box as a border and draw dashed line for selection
  QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);

  QPen myPen = pen;
  myPen.setStyle(Qt::DashLine);
  myPen.setWidth(0.3);
  painter->setPen(myPen);

  // Draw Label
  QString name = QString::fromAscii(this->getViewObject()->Label.getValue());

  QFont font;
  font.setFamily(QString::fromAscii("osifont")); // Set to generic sans-serif font
  font.setPointSize(5.f);
  painter->setFont(font);
  QFontMetrics fm(font);

  QPointF pos = box.center();
  pos.setY(box.bottom());
  pos.setX(pos.x() - fm.width(name) / 2.);

  painter->drawText(pos, name);
  painter->drawRect(box);

  painter->restore();
}

void QGraphicsItemViewPart::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;

    if(borderVisible){
         this->drawBorder(painter);
    }
    QGraphicsItemView::paint(painter, &myOption, widget);
}

#include "moc_QGraphicsItemViewPart.cpp"


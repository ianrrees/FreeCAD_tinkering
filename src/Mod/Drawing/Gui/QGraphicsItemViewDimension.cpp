/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
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
  #include <BRep_Builder.hxx>
  #include <TopoDS_Compound.hxx>
  # include <TopoDS_Shape.hxx>
  # include <TopoDS_Edge.hxx>
  # include <TopoDS.hxx>
  # include <BRepAdaptor_Curve.hxx>

  # include <QAction>
  # include <QApplication>
  # include <QContextMenuEvent>
  # include <QGraphicsScene>
  # include <QGridLayout>
  # include <QScopedPointer>
  # include <QMenu>
  # include <QMessageBox>
  # include <QMouseEvent>

  # include <QPainterPathStroker>
  # include <QPainter>
  # include <strstream>
  # include <math.h>
  # include <QGraphicsPathItem>
  # include <QGraphicsTextItem>
#endif

# include <Base/Console.h>
# include <Base/Exception.h>
# include <Gui/Command.h>

# include <Mod/Part/App/PartFeature.h>

# include <Mod/Drawing/App/FeatureViewDimension.h>
# include <Mod/Drawing/App/FeatureViewPart.h>

# include "QGraphicsItemViewDimension.h"
# include "QGraphicsItemArrow.h"

using namespace DrawingGui;


QGraphicsItemDatumLabel::QGraphicsItemDatumLabel(int ref, QGraphicsScene *scene  ) : reference(ref)
{
    if(scene) {
        scene->addItem(this);
    }
    this->posX = 0;
    this->posY = 0;

    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    this->setFlag(ItemSendsGeometryChanges, true);
    this->setFlag(ItemIsMovable, true);
    this->setFlag(ItemIsSelectable, true);
    this->setAcceptHoverEvents(true);
}

QVariant QGraphicsItemDatumLabel::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        // value is the new position.
        if(isSelected()) {
            Q_EMIT selected(true);
            this->setDefaultTextColor(Qt::blue);
        } else {
            Q_EMIT selected(false);
            this->setDefaultTextColor(Qt::black);
        }
        update();
    } else if(change == ItemPositionHasChanged && scene()) {
        updatePos();
        Q_EMIT dragging();
    }

    return QGraphicsItem::itemChange(change, value);
}

void QGraphicsItemDatumLabel::setPosition(const double &x, const double &y)
{
    // Set the actual QT Coordinates by setting at qt origin rathern than bbox center
    this->setPos(x -  this->boundingRect().width() / 2., y - this->boundingRect().height() / 2.);
}

void QGraphicsItemDatumLabel::updatePos()
{
    this->posX = this->x() + this->boundingRect().width() / 2.;
    this->posY = this->y() + this->boundingRect().height() / 2.;
}

void QGraphicsItemDatumLabel::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_EMIT hover(true);
    this->setDefaultTextColor(Qt::blue);
    update();
}

void QGraphicsItemDatumLabel::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());
    assert(view != 0);

    Q_EMIT hover(false);
    if(!isSelected() && !view->isSelected()) {
        this->setDefaultTextColor(Qt::black);
        update();
    }
}

void QGraphicsItemDatumLabel::mouseReleaseEvent( QGraphicsSceneMouseEvent * event)
{
    if(scene() && this == scene()->mouseGrabberItem()) {
        Q_EMIT dragFinished();
    }
    QGraphicsItem::mouseReleaseEvent(event);
}


void QGraphicsItemDatumLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;
    QGraphicsTextItem::paint(painter, &myOption, widget);
}

QGraphicsItemViewDimension::QGraphicsItemViewDimension(const QPoint &pos, QGraphicsScene *scene) :QGraphicsItemView(pos, scene)
{
    setHandlesChildEvents(false);
    this->setFlag(QGraphicsItem::ItemIsMovable, false);
    this->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    QGraphicsItemDatumLabel *dlabel = new QGraphicsItemDatumLabel();
    QGraphicsPathItem *arrws        = new QGraphicsPathItem();
    QGraphicsPathItem *clines       = new QGraphicsPathItem();

    this->datumLabel  = dlabel;
    this->arrows      = arrws;
    this->centreLines = clines;

    // connecting the needed slots and signals
    QObject::connect(
        dlabel, SIGNAL(dragging()),
        this  , SLOT  (datumLabelDragged()));

    QObject::connect(
        dlabel, SIGNAL(dragFinished()),
        this  , SLOT  (datumLabelDragFinished()));

    QObject::connect(
        dlabel, SIGNAL(selected(bool)),
        this  , SLOT  (select(bool)));

    QObject::connect(
        dlabel, SIGNAL(hover(bool)),
        this  , SLOT  (hover(bool)));

    this->pen.setCosmetic(true);
    this->pen.setWidthF(1.);



    this->addToGroup(arrows);
    this->addToGroup(datumLabel);
    this->addToGroup(centreLines);
}

QGraphicsItemViewDimension::~QGraphicsItemViewDimension()
{
    QGraphicsItemDatumLabel *item = static_cast<QGraphicsItemDatumLabel *>(datumLabel);
    item->deleteLater();

    clearProjectionCache();
}

void QGraphicsItemViewDimension::setViewPartFeature(Drawing::FeatureViewDimension *obj)
{
    if(obj == 0)
        return;

    this->setViewFeature(static_cast<Drawing::FeatureView *>(obj));

    // Set the QGraphicsItemGroup Properties based on the FeatureView
    float x = obj->X.getValue();
    float y = obj->Y.getValue();

    QGraphicsItemDatumLabel *dLabel = static_cast<QGraphicsItemDatumLabel *>(datumLabel);

    dLabel->setPosition(x, y);

    updateDim();
    this->draw();
    Q_EMIT dirty();
}
void QGraphicsItemViewDimension::clearProjectionCache()
{
    for(std::vector<DrawingGeometry::BaseGeom *>::iterator it = projGeom.begin(); it != projGeom.end(); ++it) {
        delete *it;
    }

    projGeom.clear();
}

void QGraphicsItemViewDimension::select(bool state)
{
    this->setSelected(state);
    draw();
}

void QGraphicsItemViewDimension::hover(bool state)
{
    this->hasHover = state;
    draw();
}

void QGraphicsItemViewDimension::updateView(bool update)
{
          // Iterate
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId()))
        return;
    Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension*>(this->getViewObject());

    std::vector<App::DocumentObject *> refs = dim->References.getValues();

  if(update ||
       dim->References.isTouched() ||
       dim->ProjDirection.isTouched()) {
        // Reset the cache;
        clearProjectionCache();
    }

    for(std::vector<App::DocumentObject *>::iterator it = refs.begin(); it != refs.end(); ++it) {
      if((*it)->isTouched()) {
          clearProjectionCache();
          break;
      }
    }

    QGraphicsItemDatumLabel *dLabel = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);

    // Identify what changed to prevent complete redraw
    if(dim->Fontsize.isTouched() ||
       dim->Font.isTouched()) {


        QFont font = dLabel->font();
        font.setPointSizeF(dim->Fontsize.getValue());
        font.setFamily(QString::fromAscii(dim->Font.getValue()));

        dLabel->setFont(font);
        dLabel->updatePos();

    } else if(dim->X.isTouched() ||
              dim->Y.isTouched()) {

        dLabel->setPosition(dim->X.getValue(), dim->Y.getValue());
        updateDim();

    } else {
        updateDim();
    }

    draw();

    Q_EMIT dirty();
}

void QGraphicsItemViewDimension::updateDim()
{
    // For now assume only show absolute dimension values
    bool absolute = true;

    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId()))
        return;

    const Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getViewObject());

    QString str = QString::fromStdString(dim->getContent()); //QString::number((absolute) ? fabs(dim->getValue()) : dim->getValue(), 'f', dim->Precision.getValue());

    QGraphicsItemDatumLabel *dLabel = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);

    // TODO eventually dimension should be individually customisable with placeholders, this should later be modularised
    const char *dimType = dim->Type.getValueAsString();


    QFont font = dLabel->font();
    font.setPointSizeF(dim->Fontsize.getValue());
    font.setFamily(QString::fromAscii(dim->Font.getValue()));

    dLabel->setPlainText(str);
    dLabel->setFont(font);
    dLabel->updatePos();
}

void QGraphicsItemViewDimension::datumLabelDragged()
{
    draw();
}

void QGraphicsItemViewDimension::datumLabelDragFinished()
{

    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId()))
        return;

    Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getViewObject());
    QGraphicsItemDatumLabel *datumLbl = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);

    double x = datumLbl->X(),
           y = datumLbl->Y();
    Gui::Command::openCommand("Drag Dimension");
    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.X = %f", dim->getNameInDocument(), x);
    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.%s.Y = %f", dim->getNameInDocument(), y);
    Gui::Command::commitCommand();
}


void QGraphicsItemViewDimension::draw()
{
    // Iterate
    if(this->getViewObject() == 0 || !this->getViewObject()->isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId()))
        return;

    Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getViewObject());
    QGraphicsItemDatumLabel *lbl = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);

    pen.setStyle(Qt::SolidLine);

    // Crude method of determining state [TODO] improve
    if(this->isSelected() || this->hasHover) {
        pen.setColor(QColor(Qt::blue));
    } else {
        pen.setColor(QColor(Qt::black));
    }

    QString str = lbl->toPlainText();
    Base::Vector3d labelPos(lbl->X(), lbl->Y(), 0);

    //Relcalculate the measurement based on references stored.
    const std::vector<App::DocumentObject*> &objects = dim->References.getValues();
    const std::vector<std::string> &SubNames         = dim->References.getSubValues();

    const char *dimType = dim->Type.getValueAsString();

    if(strcmp(dimType, "Distance") == 0 ||
       strcmp(dimType, "DistanceX") == 0 ||
       strcmp(dimType, "DistanceY") == 0) {

        Base::Vector3d p1, p2;
        if(dim->References.getValues().size() == 1 && SubNames[0].substr(0,4) == "Edge") {
            // Assuming currently just edge

            const Drawing::FeatureViewPart *refObj = static_cast<const Drawing::FeatureViewPart*>(objects[0]);
            int idx = std::atoi(SubNames[0].substr(4,4000).c_str());

            // Use the cached value
            if(projGeom.size() != 1 || !projGeom.at(0)) {
                clearProjectionCache();
                projGeom.push_back(refObj->getCompleteEdge(idx));
            }

            if(projGeom.at(0) && projGeom.at(0)->geomType == DrawingGeometry::GENERIC ) {
                DrawingGeometry::Generic *gen = static_cast<DrawingGeometry::Generic *>(projGeom.at(0));
                Base::Vector2D pnt1 = gen->points.at(0);
                Base::Vector2D pnt2 = gen->points.at(1);
                p1 = Base::Vector3d(pnt1.fX, pnt1.fY, 0.);
                p2 = Base::Vector3d(pnt2.fX, pnt2.fY, 0.);

            } else {
                clearProjectionCache();
                throw Base::Exception("Original edge not found or is invalid type");
            }

        } else if(dim->References.getValues().size() == 2 &&
                  SubNames[0].substr(0,6) == "Vertex" &&
                  SubNames[1].substr(0,6) == "Vertex") {
            // Point to Point Dimension
            const Drawing::FeatureViewPart *refObj = static_cast<const Drawing::FeatureViewPart*>(objects[0]);
            int idx = std::atoi(SubNames[0].substr(6,4000).c_str());
            int idx2 = std::atoi(SubNames[1].substr(6,4000).c_str());

            DrawingGeometry::Vertex *v1 = refObj->getVertex(idx);
            DrawingGeometry::Vertex *v2 = refObj->getVertex(idx2);
            p1 = Base::Vector3d (v1->pnt.fX, v1->pnt.fY, 0.);
            p2 = Base::Vector3d (v2->pnt.fX, v2->pnt.fY, 0.);

            // Do some house keeping
            delete v1; v1 = 0;
            delete v2; v2 = 0;

        } else if(dim->References.getValues().size() == 2 &&
            SubNames[0].substr(0,4) == "Edge" &&
            SubNames[1].substr(0,4) == "Edge") {
            // Point to Point Dimension
            const Drawing::FeatureViewPart *refObj = static_cast<const Drawing::FeatureViewPart*>(objects[0]);
            int idx = std::atoi(SubNames[0].substr(4,4000).c_str());
            int idx2 = std::atoi(SubNames[1].substr(4,4000).c_str());

            // Use the cached value or gather projected edges
            if(projGeom.size() != 2 || !projGeom.at(0) || !projGeom.at(0)) {
                clearProjectionCache();
                projGeom.push_back(refObj->getCompleteEdge(idx));
                projGeom.push_back(refObj->getCompleteEdge(idx2));
            }

            if(projGeom.at(0) && projGeom.at(0)->geomType == DrawingGeometry::GENERIC ||
               projGeom.at(1) && projGeom.at(1)->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(projGeom.at(0));
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(projGeom.at(1));

                // Get Points for line
                Base::Vector2D pnt1, pnt2;
                Base::Vector3d p1S, p1E, p2S, p2E;
                pnt1 = gen1->points.at(0);
                pnt2 = gen1->points.at(1);

                p1S = Base::Vector3d(pnt1.fX, pnt1.fY, 0);
                p1E = Base::Vector3d(pnt2.fX, pnt2.fY, 0);

                pnt1 = gen2->points.at(0);
                pnt2 = gen2->points.at(1);

                p2S = Base::Vector3d(pnt1.fX, pnt1.fY, 0);
                p2E = Base::Vector3d(pnt2.fX, pnt2.fY, 0);

                // Calaculate dot product using p1S as reference
                Base::Vector3d lin1 = p1E - p1S;
                Base::Vector3d lin2 = p2E - p2S;

                Base::Vector3d labelV1 = labelPos - p1S;
                Base::Vector3d labelV2 = labelPos - p2S;

                //Sort first edges
                if(lin1.x * labelV1.x + lin1.y * labelV1.y > 0.)
                    p1 = p1E;
                else
                    p1 = p1S;

                if(lin2.x * labelV2.x + lin2.y * labelV2.y > 0.)
                    p2 = p2E;
                else
                    p2 = p2S;

            } else {
                throw Base::Exception("Invalid reference for dimension type");
            }
        }

        Base::Vector3d dir, norm;

        if (strcmp(dimType, "Distance") == 0 ) {
            dir = (p2-p1);
        } else if (strcmp(dimType, "DistanceX") == 0 ) {
            dir = Base::Vector3d ( (p2[0] - p1[0] >= FLT_EPSILON) ? 1 : -1, 0, 0);
        } else if (strcmp(dimType, "DistanceY") == 0 ) {
            dir = Base::Vector3d (0, (p2[1] - p1[1] >= FLT_EPSILON) ? 1 : -1, 0);
        }

        dir.Normalize();
        norm = Base::Vector3d (-dir[1],dir[0], 0);

        // Get magnitude of angle between horizontal
        float angle = atan2f(dir[1],dir[0]);
        bool flip=false;
        if (angle > M_PI_2+M_PI/12) {
            angle -= (float)M_PI;
            flip = true;
        } else if (angle <= -M_PI_2+M_PI/12) {
            angle += (float)M_PI;
            flip = true;
        }

        // when the datum line is not parallel to p1-p2 the projection of
        // p1-p2 on norm is not zero, p2 is considered as reference and p1
        // is replaced by its projection p1_
        float normproj12 = (p2-p1)[0] * norm[0] + (p2-p1)[1] * norm[1];
        Base::Vector3d p1_ = p1 + norm * normproj12;

        Base::Vector3d midpos = (p1_ + p2) / 2;

        QFontMetrics fm(lbl->font());
        int w = fm.width(str);
        int h = fm.height();

        Base::Vector3d vec = labelPos - p2;
        float length = vec.x * norm.x + vec.y * norm.y;

        float margin = 3.f;
        float scaler = 1.;

        float offset1 = (length + normproj12 < 0) ? -margin : margin;
        float offset2 = (length < 0) ? -margin : margin;

        Base::Vector3d perp1 = p1_ + norm * (length + offset1 * scaler);
        Base::Vector3d perp2 = p2  + norm * (length + offset2 * scaler);

        // Calculate the coordinates for the parallel datum lines
        Base::Vector3d  par1 = p1_ + norm * length;

        Base::Vector3d  par2 = labelPos - dir * (w / 2 + margin);
        Base::Vector3d  par3 = labelPos + dir * (w / 2 + margin);
        Base::Vector3d  par4 = p2  + norm * length;

        // Add a small margin
        //p1_ += norm * margin * 0.5;
       // p2  += norm * margin * 0.5;

        bool flipTriang = false;

        Base::Vector3d del1 = (par3-par1);
        Base::Vector3d del2 = (par2-par1);
        float dot1 = del1.x * dir.x + del1.y * dir.y;
        float dot2 = del2.x * dir.x + del2.y * dir.y;

        //Compare to see if datum label is larger than dimension
        if (dot1 > (par4 - par1).Length()) {
            // Increase Margin to improve visability
            float tmpMargin = 10.f * scaler;
            par3 = par4;
            if(dot2 > (par4 - par1).Length()) {
                par3 = par2;
                par2 = par1 - dir * tmpMargin;
                flipTriang = true;
            }
        } else if (dot2 < 0.f) {
            float tmpMargin = 10.f * scaler;
            par2 = par1;
            if(dot1 < 0.f) {
                par2 = par3;
                par3 = par4 + dir * tmpMargin;
                flipTriang = true;
            }
        }



        // Perp Lines
        QPainterPath path;
        path.moveTo(p1.x, p1.y);
        path.lineTo(perp1.x, perp1.y);

        path.moveTo(p2.x, p2.y);
        path.lineTo(perp2.x, perp2.y);

        // Parallel Lines
        path.moveTo(par1.x, par1.y);
        path.lineTo(par2.x, par2.y);

        path.moveTo(par3.x, par3.y);
        path.lineTo(par4.x, par4.y);

        QGraphicsPathItem *arrw = dynamic_cast<QGraphicsPathItem *> (this->arrows);
        arrw->setPath(path);
        arrw->setPen(pen);

        // Note Bounding Box size is not the same width or height as text (only used for finding center)
        float bbX  = lbl->boundingRect().width();
        float bbY = lbl->boundingRect().height();
        lbl->setTransformOriginPoint(bbX / 2, bbY /2);
        lbl->setRotation(angle * 180 / M_PI);


        if(arw.size() != 2) {
            prepareGeometryChange();
            for(std::vector<QGraphicsItem *>::iterator it = arw.begin(); it != arw.end(); ++it) {
                this->removeFromGroup(*it);
                delete (*it);
            }
            arw.clear();

            // These items are added to the scene-graph so should be handled by the canvas
            QGraphicsItemArrow *ar1 = new QGraphicsItemArrow();
            QGraphicsItemArrow *ar2 = new QGraphicsItemArrow();
            arw.push_back(ar1);
            arw.push_back(ar2);

            ar1->draw();
            ar2->flip(true);
            ar2->draw();

            this->addToGroup(arw.at(0));
            this->addToGroup(arw.at(1));
        }

        QGraphicsItemArrow *ar1 = dynamic_cast<QGraphicsItemArrow *>(arw.at(0));
        QGraphicsItemArrow *ar2 = dynamic_cast<QGraphicsItemArrow *>(arw.at(1));

        angle = atan2f(dir[1],dir[0]);
        float arrowAngle = angle * 180 / M_PI;
        arrowAngle -= 180.;
        if(flipTriang){
            ar1->setRotation(arrowAngle + 180.);
            ar2->setRotation(arrowAngle + 180.);
        } else {
            ar1->setRotation(arrowAngle);
            ar2->setRotation(arrowAngle);
        }

        ar1->setPos(par1[0], par1[1]);
        ar2->setPos(par4[0], par4[1]);

        ar1->setHighlighted(isSelected() || this->hasHover);
        ar2->setHighlighted(isSelected() || this->hasHover);

    } else if(strcmp(dimType, "Diameter") == 0) {
        // Not sure whether to treat radius and diameter as the same

        Base::Vector3d p1, p2, dir, centre;
        QGraphicsItemDatumLabel *lbl = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);
        Base::Vector3d labelPos(lbl->X(), lbl->Y(), 0);

        double radius;

        if(dim->References.getValues().size() == 1 && SubNames[0].substr(0,4) == "Edge") {
            // Assuming currently just edge

            const Drawing::FeatureViewPart *refObj = static_cast<const Drawing::FeatureViewPart*>(objects[0]);
            int idx = std::atoi(SubNames[0].substr(4,4000).c_str());

            // Use the cached value if available otherwise load this
            if(projGeom.size() != 1) {
                clearProjectionCache();
                DrawingGeometry::BaseGeom *geom = refObj->getCompleteEdge(idx);

                if(!geom)
                    throw Base::Exception("Edge couldn't be found for radius / diameter dimension");

                projGeom.push_back(geom);
            }

            if(projGeom.at(0) &&
               (projGeom.at(0)->geomType == DrawingGeometry::CIRCLE || projGeom.at(0)->geomType == DrawingGeometry::ARCOFCIRCLE)) {
                  DrawingGeometry::Circle *circ = static_cast<DrawingGeometry::Circle *>(projGeom.at(0));
                  Base::Vector2D pnt1 = circ->center;
                  radius = circ->radius;
                  centre = Base::Vector3d (pnt1.fX, pnt1.fY, 0); // Center point
            } else {
                clearProjectionCache();
                throw Base::Exception("Original edge not found or is invalid type");
            }
        } else {
            throw Base::Exception("Invalid reference for dimension type");
        }

        // Note Bounding Box size is not the same width or height as text (only used for finding center)
        float bbX  = lbl->boundingRect().width();
        float bbY = lbl->boundingRect().height();

        // TODO consider modifying behaviour of SoDatumLabel so position is at center
        // Orientate Position to be at the center of the datumLabel
//         labelPos += 0.5 * Base::Vector3d(bbX, bbY, 0.f);
        dir = (labelPos - centre).Normalize();

        // Get magnitude of angle between horizontal
        float angle = atan2f(dir[1],dir[0]);
        bool flip=false;
        if (angle > M_PI_2+M_PI/12) {
            angle -= (float)M_PI;
            flip = true;
        } else if (angle <= -M_PI_2+M_PI/12) {
            angle += (float)M_PI;
            flip = true;
        }

        float s = sin(angle);
        float c = cos(angle);
        /*

        Base::Console().Log("angle (%f, %f), bbx %f, bby %f", s,c,bbX, bbY);*/
        // Note QGraphicsTextItem takes coordinate system from TOP LEFT - transfer to center
        // Create new coordinate system based around x,y
//         labelPos += 0.5 * Base::Vector3d(bbX * c - bbY * s, bbX * s + bbY * c, 0.f);
//         dir = (labelPos - p1).Normalize();
//
//         angle = atan2f(dir[1],dir[0]);
//
        p2 = centre + dir * radius;
        p1 = centre - dir * radius;

        QFontMetrics fm(lbl->font());

        int w = fm.width(str);
        int h = fm.height();

        float margin = 5.f;

        // Calculate the points
        Base::Vector3d pnt1 = labelPos - dir * (margin + w / 2);
        Base::Vector3d pnt2 = labelPos + dir * (margin + w / 2);

        bool outerPlacement = false;
        if ((labelPos-centre).Length() > radius) {
            outerPlacement = true;
        }

        // Reset transformation origin for datum label
        lbl->setTransformOriginPoint(bbX / 2, bbY /2);

        int posMode = 0; // 0 - Position freely
                         // 1 - Snap Vertically
                         // 2 - Snap
        QPainterPath path;

        if(outerPlacement) {

            // Select whether to snap vertically or hoziontally given tolerance
            Base::Vector3d v = (labelPos-p1);

            double angle = atan2(v.y, v.x);
            double tolerance = 15.0; //deg

            tolerance *= M_PI / 180;
            if( (angle > -tolerance && angle < tolerance) ||
                (angle > (M_PI - tolerance) || angle < (-M_PI + tolerance)) ) {
                // Snap horizontally
                  posMode = 2;
            } else if( (angle < ( M_PI / 2. + tolerance) && angle > ( M_PI / 2. - tolerance)) ||
                       (angle < (-M_PI / 2. + tolerance) && angle > (-M_PI / 2. - tolerance)) ) {
                // Snap vertically
                posMode = 1;
            }

            if(posMode == 1) {

                // Snapped Vertically
                float tip = (labelPos.y > centre.y) ? margin: -margin;
                tip *= 0.5;

                p1.x = centre.x - radius;
                p1.y = labelPos.y;

                p2.x = centre.x + radius;
                p2.y = labelPos.y;

                pnt1 = labelPos;
                pnt1.x -= (margin + w / 2);

                pnt2 = labelPos;
                pnt2.x += (margin + w / 2);

                // Extension lines
                path.moveTo(centre.x - radius, centre.y);
                path.lineTo(p1[0], p1[1] + tip);

                // Left Arrow
                path.moveTo(p1[0], p1[1]);
                path.lineTo(pnt1[0], pnt1[1]);

                // Extension lines
                path.moveTo(centre.x + radius, centre.y);
                path.lineTo(p2[0], p2[1] + tip);

                // Right arrow
                path.moveTo(pnt2[0], pnt2[1]);
                path.lineTo(p2[0], p2[1]);

                lbl->setRotation(0.);

            } else if(posMode == 2) {
                // Snapped Horizontally

                float tip = (labelPos.x > centre.x) ? margin: -margin;
                tip *= 0.5;

                p1.y = centre.y - radius;
                p1.x = labelPos.x;

                p2.y = centre.y + radius;
                p2.x = labelPos.x;

                pnt1 = labelPos;
                pnt1.y -= (margin + w / 2);

                pnt2 = labelPos;
                pnt2.y += (margin + w / 2);

                // Extension lines
                path.moveTo(centre.x, centre.y  - radius);
                path.lineTo(p1[0] + tip, p1[1]);

                path.moveTo(p1[0], p1[1]);
                path.lineTo(pnt1[0], pnt1[1]);

                // Extension lines
                path.moveTo(centre.x, centre.y  + radius);
                path.lineTo(p2[0] + tip, p2[1]);

                path.moveTo(pnt2[0], pnt2[1]);
                path.lineTo(p2[0], p2[1]);

                lbl->setRotation(90.);

            } else {
                float tip = (margin + w / 2);
                tip = (labelPos.x < centre.x) ? tip : -tip;

                p1 = labelPos;
                p1.x += tip;


                Base::Vector3d p3 = p1;
                p3.x += (labelPos.x < centre.x) ? margin : - margin;

                p2 = centre + (p3 - centre).Normalize() * radius;

                path.moveTo(p1[0], p1[1]);
                path.lineTo(p3[0], p3[1]);

                path.lineTo(p2[0], p2[1]);

                lbl->setRotation(0.);
            }
        } else {
            lbl->setRotation(angle * 180 / M_PI);

            path.moveTo(p1[0], p1[1]);
            path.lineTo(pnt1[0], pnt1[1]);

            path.moveTo(pnt2[0], pnt2[1]);
            path.lineTo(p2[0], p2[1]);
        }

        QGraphicsPathItem *arrw = dynamic_cast<QGraphicsPathItem *> (this->arrows);
        arrw->setPath(path);
        arrw->setPen(pen);

        // Add or remove centre lines
        QGraphicsPathItem *clines = dynamic_cast<QGraphicsPathItem *> (this->centreLines);
        QPainterPath clpath;

        if(dim->CentreLines.getValue()) {
            // Add centre lines to the circle

            double clDist = margin; // Centre Line Size
            if( margin / radius  > 0.2) {
                // Tolerance if centre line is greater than 0.3x radius then set to limit
                clDist = radius * 0.2;
            }
            // Vertical Line
            clpath.moveTo(centre.x, centre.y + clDist);
            clpath.lineTo(centre.x, centre.y - clDist);

            // Vertical Line
            clpath.moveTo(centre.x - clDist, centre.y);
            clpath.lineTo(centre.x + clDist, centre.y);

            QPen clPen(QColor(128,128,128));
            clines->setPen(clPen);
        }

        clines->setPath(clpath);

        // Create Two Arrows always
        if(arw.size() != 2) {
            prepareGeometryChange();
            for(std::vector<QGraphicsItem *>::iterator it = arw.begin(); it != arw.end(); ++it) {
                this->removeFromGroup(*it);
                delete (*it);
            }
            arw.clear();

            // These items are added to the scene-graph so should be handled by the canvas
            QGraphicsItemArrow *ar1 = new QGraphicsItemArrow();
            QGraphicsItemArrow *ar2 = new QGraphicsItemArrow();
            arw.push_back(ar1);
            arw.push_back(ar2);

            ar1->draw();
            ar2->flip(true);
            ar2->draw();
            this->addToGroup(arw.at(0));
            this->addToGroup(arw.at(1));
        }

        QGraphicsItemArrow *ar1 = dynamic_cast<QGraphicsItemArrow *>(arw.at(0));
        QGraphicsItemArrow *ar2 = dynamic_cast<QGraphicsItemArrow *>(arw.at(1));

        Base::Vector3d ar1Pos = p1 + dir * radius;
        float arAngle = atan2(dir.y, dir.x) * 180 / M_PI;

        ar1->setHighlighted(isSelected() || this->hasHover);
        ar2->setHighlighted(isSelected() || this->hasHover);
        ar2->show();

        if(outerPlacement) {
            if(posMode > 0) {
                  ar1->setPos(p2.x, p2.y);
                  ar2->setPos(p1.x, p1.y);
                  ar1->setRotation((posMode == 2) ? 90 : 0);
                  ar2->setRotation((posMode == 2) ? 90 : 0);
            } else {
                Base::Vector3d vec = (p2 - centre).Normalize();
                float arAngle = atan2(-vec.y, -vec.x) * 180 / M_PI;
                ar1->setPos(p2.x, p2.y);
                ar1->setRotation(arAngle);
                ar2->hide();
            }
        } else {
            ar1->setRotation(arAngle);
            ar2->setRotation(arAngle);

            ar1->setPos(p2.x, p2.y);
            ar2->show();
            ar2->setPos(p1.x, p1.y);
        }


    } else if(strcmp(dimType, "Radius") == 0) {
        // Not sure whether to treat radius and diameter as the same

        Base::Vector3d p1, p2, dir;
        QGraphicsItemDatumLabel *lbl = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);
        Base::Vector3d labelPos(lbl->X(), lbl->Y(), 0);

        double radius;

        if(dim->References.getValues().size() == 1 && SubNames[0].substr(0,4) == "Edge") {
            // Assuming currently just edge

            const Drawing::FeatureViewPart *refObj = static_cast<const Drawing::FeatureViewPart*>(objects[0]);
            int idx = std::atoi(SubNames[0].substr(4,4000).c_str());

            // Use the cached value if available otherwise load this
            if(projGeom.size() != 1) {
                clearProjectionCache();
                DrawingGeometry::BaseGeom *geom = refObj->getCompleteEdge(idx);

                if(!geom)
                    throw Base::Exception("Edge couldn't be found for radius / diameter dimension");

                projGeom.push_back(geom);
            }

            if(projGeom.at(0) &&
               (projGeom.at(0)->geomType == DrawingGeometry::CIRCLE || projGeom.at(0)->geomType == DrawingGeometry::ARCOFCIRCLE)) {
                  DrawingGeometry::Circle *circ = static_cast<DrawingGeometry::Circle *>(projGeom.at(0));
                  Base::Vector2D pnt1 = circ->center;
                  radius = circ->radius;
                  p1 = Base::Vector3d (pnt1.fX, pnt1.fY, 0);
            } else {
                clearProjectionCache();
                throw Base::Exception("Original edge not found or is invalid type");
            }
        } else {
            throw Base::Exception("Invalid reference for dimension type");
        }

        // Note Bounding Box size is not the same width or height as text (only used for finding center)
        float bbX  = lbl->boundingRect().width();
        float bbY = lbl->boundingRect().height();

        // TODO consider modifying behaviour of SoDatumLabel so position is at center
        // Orientate Position to be at the center of the datumLabel
//         labelPos += 0.5 * Base::Vector3d(bbX, bbY, 0.f);
        dir = (labelPos - p1).Normalize();

        // Get magnitude of angle between horizontal
        float angle = atan2f(dir[1],dir[0]);
        bool flip=false;
        if (angle > M_PI_2+M_PI/12) {
            angle -= (float)M_PI;
            flip = true;
        } else if (angle <= -M_PI_2+M_PI/12) {
            angle += (float)M_PI;
            flip = true;
        }

        float s = sin(angle);
        float c = cos(angle);
        /*

        Base::Console().Log("angle (%f, %f), bbx %f, bby %f", s,c,bbX, bbY);*/
        // Note QGraphicsTextItem takes coordinate system from TOP LEFT - transfer to center
        // Create new coordinate system based around x,y
//         labelPos += 0.5 * Base::Vector3d(bbX * c - bbY * s, bbX * s + bbY * c, 0.f);
//         dir = (labelPos - p1).Normalize();
//
//         angle = atan2f(dir[1],dir[0]);
//
        p2 = p1 + dir * radius;

        QFontMetrics fm(lbl->font());

        int w = fm.width(str);
        int h = fm.height();

        float margin = 5.f;

//         // Create the arrowhead
//         SbVec3f ar0  = p2;
//         SbVec3f ar1  = p2 - dir * 0.866f * 2 * margin;
//         SbVec3f ar2  = ar1 + norm * margin;
//         ar1 -= norm * margin;

        // Calculate the points
        Base::Vector3d pnt1 = labelPos - dir * (margin + w / 2);
        Base::Vector3d pnt2 = labelPos + dir * (margin + w / 2);

        bool outerPlacement = false;
        if ((labelPos-p1).Length() > (p2-p1).Length()) {
            p2 = pnt2;
            outerPlacement = true;
        }

        QPainterPath path;
        path.moveTo(p1[0], p1[1]);
        path.lineTo(pnt1[0], pnt1[1]);

        path.moveTo(pnt2[0], pnt2[1]);
        path.lineTo(p2[0], p2[1]);

        QGraphicsPathItem *arrw = dynamic_cast<QGraphicsPathItem *> (this->arrows);
        arrw->setPath(path);
        arrw->setPen(pen);

        lbl->setTransformOriginPoint(bbX / 2, bbY /2);
        lbl->setRotation(angle * 180 / M_PI);

        if(outerPlacement) {
            if(arw.size() != 1) {
                prepareGeometryChange();
                for(std::vector<QGraphicsItem *>::iterator it = arw.begin(); it != arw.end(); ++it) {
                    this->removeFromGroup(*it);
                    delete (*it);
                }
                arw.clear();

                // These items are added to the scene-graph so should be handled by the canvas
                QGraphicsItemArrow *ar1 = new QGraphicsItemArrow();
                arw.push_back(ar1);

                ar1->draw();
                this->addToGroup(arw.at(0));
            }
        } else {
            // Create Two Arrows
            if(arw.size() != 2) {
                prepareGeometryChange();
                for(std::vector<QGraphicsItem *>::iterator it = arw.begin(); it != arw.end(); ++it) {
                    this->removeFromGroup(*it);
                    delete (*it);
                }
                arw.clear();

                // These items are added to the scene-graph so should be handled by the canvas
                QGraphicsItemArrow *ar1 = new QGraphicsItemArrow();
                QGraphicsItemArrow *ar2 = new QGraphicsItemArrow();
                arw.push_back(ar1);
                arw.push_back(ar2);

                ar1->draw();
                ar2->flip(true);
                ar2->draw();
                this->addToGroup(arw.at(0));
                this->addToGroup(arw.at(1));
            }
        }


        // Add or remove centre lines
        QGraphicsPathItem *clines = dynamic_cast<QGraphicsPathItem *> (this->centreLines);
        QPainterPath clpath;

        if(dim->CentreLines.getValue()) {
            // Add centre lines to the circle

            double clDist = margin; // Centre Line Size
            if( margin / radius  > 0.2) {
                // Tolerance if centre line is greater than 0.3x radius then set to limit
                clDist = radius * 0.2;
            }
            // Vertical Line
            clpath.moveTo(p1.x, p1.y + clDist);
            clpath.lineTo(p1.x, p1.y - clDist);

            // Vertical Line
            clpath.moveTo(p1.x - clDist, p1.y);
            clpath.lineTo(p1.x + clDist, p1.y);

            QPen clPen(QColor(128,128,128));
            clines->setPen(clPen);
        }

        clines->setPath(clpath);

        QGraphicsItemArrow *ar1 = dynamic_cast<QGraphicsItemArrow *>(arw.at(0));

        Base::Vector3d ar1Pos = p1 + dir * radius;
        float arAngle = atan2(dir.y, dir.x) * 180 / M_PI;

        ar1->setPos(ar1Pos.x, ar1Pos.y);
        ar1->setRotation(arAngle);
        ar1->setHighlighted(isSelected() || this->hasHover);

        if(!outerPlacement) {
            QGraphicsItemArrow *ar2 = dynamic_cast<QGraphicsItemArrow *>(arw.at(1));
            ar2->setPos(p1.x, p1.y);
            ar2->setRotation(arAngle);
            ar2->setHighlighted(isSelected() || this->hasHover);
        }
    } else if(strcmp(dimType, "Angle") == 0) {

        // Only use two straight line edeges for angle
        if(dim->References.getValues().size() == 2 &&
            SubNames[0].substr(0,4) == "Edge" &&
            SubNames[1].substr(0,4) == "Edge") {
            // Point to Point Dimension
            const Drawing::FeatureViewPart *refObj = static_cast<const Drawing::FeatureViewPart*>(objects[0]);
            int idx = std::atoi(SubNames[0].substr(4,4000).c_str());
            int idx2 = std::atoi(SubNames[1].substr(4,4000).c_str());

            // Use the cached value or gather projected edges
            if(projGeom.size() != 2 || !projGeom.at(0) || !projGeom.at(0)) {
                clearProjectionCache();
                projGeom.push_back(refObj->getCompleteEdge(idx));
                projGeom.push_back(refObj->getCompleteEdge(idx2));
            }

            if(projGeom.at(0) && projGeom.at(0)->geomType == DrawingGeometry::GENERIC ||
               projGeom.at(1) && projGeom.at(1)->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(projGeom.at(0));
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(projGeom.at(1));

                // Get Points for line
                Base::Vector2D pnt1, pnt2;
                Base::Vector3d p1S, p1E, p2S, p2E;
                pnt1 = gen1->points.at(0);
                pnt2 = gen1->points.at(1);

                p1S = Base::Vector3d(pnt1.fX, pnt1.fY, 0);
                p1E = Base::Vector3d(pnt2.fX, pnt2.fY, 0);

                pnt1 = gen2->points.at(0);
                pnt2 = gen2->points.at(1);

                p2S = Base::Vector3d(pnt1.fX, pnt1.fY, 0);
                p2E = Base::Vector3d(pnt2.fX, pnt2.fY, 0);

                Base::Vector3d dir1 = p1E - p1S;
                Base::Vector3d dir2 = p2E - p2S;

                double det = dir1.x*dir2.y - dir1.y*dir2.x;
                if ((det > 0 ? det : -det) < 1e-10)
                    return;
                double c1 = dir1.y*p1S.x - dir1.x*p1S.y;
                double c2 = dir2.y*p2S.x - dir2.x*p2S.y;
                double x = (dir1.x*c2 - dir2.x*c1)/det;
                double y = (dir1.y*c2 - dir2.y*c1)/det;

                Base::Vector3d p0(x,y,0);

                // Get directions with outwards orientation and check if coincident
                dir1 = ((p1E - p0).Length() > (p1S - p0).Length()) ? p1E - p0 : p1S - p0;
                dir2 = ((p2E - p0).Length() > (p2S - p0).Length()) ? p2E - p0 : p2S - p0;

                // Qt y coordinates are flipped
                dir1.y *= -1.;
                dir2.y *= -1.;

                Base::Vector3d labelVec = (labelPos - p0);

                double labelangle = atan2(-labelVec.y, labelVec.x);

                double startangle = atan2(dir1.y,dir1.x);
                double range      = atan2(-dir1.y*dir2.x+dir1.x*dir2.y,
                                           dir1.x*dir2.x+dir1.y*dir2.y);

                double endangle = startangle + range;

                // Obtain the Label Position and measure the length between intersection
                QGraphicsItemDatumLabel *lbl = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);
                Base::Vector3d labelPos(lbl->X(), lbl->Y(), 0);

                float bbX  = lbl->boundingRect().width();
                float bbY  = lbl->boundingRect().height();

                // Get font height
                QFontMetrics fm(lbl->font());

                int h = fm.height();
                double length = labelVec.Length();
                length -= h * 0.6; // Adjust the length so the label isn't over the line

                // Find the end points for dim lines
                Base::Vector3d p1 = ((p1E - p0).Length() > (p1S - p0).Length()) ? p1E : p1S;
                Base::Vector3d p2 = ((p2E - p0).Length() > (p2S - p0).Length()) ? p2E : p2S;

                // add an offset from the ends (add 1mm from end)
                p1 += (p1-p0).Normalize() * 5.;
                p2 += (p2-p0).Normalize() * 5.;

                Base::Vector3d ar1Pos = p0;
                Base::Vector3d ar2Pos = p0;

                ar1Pos += Base::Vector3d(cos(startangle) * length, -sin(startangle) * length, 0.);
                ar2Pos += Base::Vector3d(cos(endangle) * length  , -sin(endangle) * length, 0.);

                // Draw the path
                QPainterPath path;

                // Only draw extension lines if outside arc
                if(length > (p1-p0).Length()) {
                    path.moveTo(p1.x, p1.y);
                    p1 = ar1Pos + (p1-p0).Normalize() * 5.;
                    path.lineTo(p1.x, p1.y);
                }

                if(length > (p2-p0).Length()) {
                    path.moveTo(p2.x, p2.y);
                    p2 = ar2Pos + (p2-p0).Normalize() * 5.;
                    path.lineTo(p2.x, p2.y);
                }


                bool isOutside = true;

                // TODO find a better solution for this. Addmitedely not tidy
                // ###############
                // Treat zero as positive to be consistent for horizontal lines
                if(std::abs(startangle) < FLT_EPSILON)
                    startangle = 0;

                 if(std::abs(endangle) < FLT_EPSILON)
                    endangle = 0;

                if(startangle >= 0 && endangle >= 0) {
                    // Both are in positive side
                  double langle = labelangle;
                  if(labelangle < 0)
                    langle += M_PI * 2;
                    if(endangle - startangle > 0) {
                        if(langle > startangle && langle < endangle)
                            isOutside = false;
                    } else {
                        if(langle < startangle && langle > endangle)
                            isOutside = false;
                    }
                } else if(startangle < 0 && endangle < 0) {
                    // Both are in positive side
                   double langle = labelangle;
                    if(labelangle > 0)
                        langle -= M_PI * 2;
                    if(endangle - startangle < 0) {
                        if(langle > endangle && langle < startangle) // clockwise
                            isOutside = false;
                    } else {
                        if(langle < endangle && langle > startangle) // anticlockwise
                            isOutside = false;
                    }
                } else if(startangle >= 0 && endangle < 0) {
                    if(labelangle < startangle && labelangle > endangle) // clockwise
                        isOutside = false;

                } else if(startangle < 0 && endangle >= 0) {
                   // Both are in positive side

                    if(labelangle > startangle && labelangle < endangle) // clockwise
                        isOutside = false;
                }

                // ###############
//                 Base::Console().Log("<%f, %f, %f>\n", startangle, endangle, labelangle);

                QRectF arcRect(p0.x - length, p0.y - length, 2. * length, 2. * length);
                path.arcMoveTo(arcRect, endangle * 180 / M_PI);
                if(isOutside) {
                    if(labelangle > endangle)
                    {
                        path.arcTo(arcRect, endangle * 180 / M_PI, (labelangle  - endangle) * 180 / M_PI); // chosen a nominal value for 10 degrees
                        path.arcMoveTo(arcRect,startangle * 180 / M_PI);
                        path.arcTo(arcRect, startangle * 180 / M_PI, -10);
                    } else {
                        path.arcTo(arcRect, endangle * 180 / M_PI, 10); // chosen a nominal value for 10 degrees
                        path.arcMoveTo(arcRect,startangle * 180 / M_PI);
                        path.arcTo(arcRect, startangle * 180 / M_PI, (labelangle - startangle) * 180 / M_PI);
                    }


                } else {
                    path.arcTo(arcRect, endangle * 180 / M_PI, -range * 180 / M_PI);
                }

                QGraphicsPathItem *arrw = dynamic_cast<QGraphicsPathItem *> (this->arrows);
                arrw->setPath(path);
                arrw->setPen(pen);

                // Add the arrows
                if(arw.size() != 2) {
                    prepareGeometryChange();
                    for(std::vector<QGraphicsItem *>::iterator it = arw.begin(); it != arw.end(); ++it) {
                        this->removeFromGroup(*it);
                        delete (*it);
                    }
                    arw.clear();

                    // These items are added to the scene-graph so should be handled by the canvas
                    QGraphicsItemArrow *ar1 = new QGraphicsItemArrow();
                    QGraphicsItemArrow *ar2 = new QGraphicsItemArrow();
                    arw.push_back(ar1);
                    arw.push_back(ar2);

                    ar1->flip(true);
                    ar1->draw();
                    ar2->draw();

                    this->addToGroup(arw.at(0));
                    this->addToGroup(arw.at(1));
                }

                QGraphicsItemArrow *ar1 = dynamic_cast<QGraphicsItemArrow *>(arw.at(0));
                QGraphicsItemArrow *ar2 = dynamic_cast<QGraphicsItemArrow *>(arw.at(1));

                Base::Vector3d norm1 = p1-p0; //(-dir1.y, dir1.x, 0.);
                Base::Vector3d norm2 = p2-p0; //(-dir2.y, dir2.x, 0.);

                Base::Vector3d avg = (norm1 + norm2) / 2.;

                norm1 = norm1.ProjToLine(avg, norm1);
                norm2 = norm2.ProjToLine(avg, norm2);

                ar1->setPos(ar1Pos.x,ar1Pos.y );
                ar2->setPos(ar2Pos.x,ar2Pos.y );

                float ar1angle = atan2(-norm1.y, -norm1.x) * 180 / M_PI;
                float ar2angle = atan2(norm2.y, norm2.x) * 180 / M_PI;

                if(isOutside) {
                    ar1->setRotation(ar1angle + 180.);
                    ar2->setRotation(ar2angle + 180.);
                } else {
                    ar1->setRotation(ar1angle);
                    ar2->setRotation(ar2angle);
                }

                ar1->setHighlighted(isSelected() || this->hasHover);
                ar2->setHighlighted(isSelected() || this->hasHover);

                // Set the angle of the datum text

                Base::Vector3d labelNorm(-labelVec.y, labelVec.x, 0.);
                double lAngle = atan2(labelNorm.y, labelNorm.x);

                if (lAngle > M_PI_2+M_PI/12) {
                    lAngle -= M_PI;
                } else if (lAngle <= -M_PI_2+M_PI/12) {
                    lAngle += M_PI;
                }

                lbl->setTransformOriginPoint(bbX / 2., bbY /2.);

                lbl->setRotation(lAngle * 180 / M_PI);

            } else {
                throw Base::Exception("Invalid reference for dimension type");
            }
        }
    }

    // Call to redraw the object
    update();
}

QVariant QGraphicsItemViewDimension::itemChange(GraphicsItemChange change, const QVariant &value)
{
   if (change == ItemSelectedHasChanged && scene()) {
        QGraphicsItemDatumLabel *dLabel = dynamic_cast<QGraphicsItemDatumLabel *>(this->datumLabel);

        if(isSelected()) {
            dLabel->setSelected(true);
        } else {
            dLabel->setSelected(false);
        }
        draw();
    }
    return QGraphicsItemView::itemChange(change, value);
}

void QGraphicsItemViewDimension::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;
    QGraphicsItemView::paint(painter, &myOption, widget);
}

#include "moc_QGraphicsItemViewDimension.cpp"

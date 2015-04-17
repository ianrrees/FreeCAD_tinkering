/***************************************************************************
 *   Copyright (c) 2012-2013 Luke Parry <l.parry@warwick.ac.uk>            *
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
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPathStroker>
#include <QStyleOptionGraphicsItem>
#include <QTextOption>
#include <QTransform>
#include <strstream>
#endif

#include <App/Application.h>
#include <App/Document.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>
#include <Gui/Selection.h>
#include <Gui/Command.h>

#include "../App/FeatureView.h"
#include "QGraphicsItemView.h"

using namespace DrawingGui;

QGraphicsItemView::QGraphicsItemView(const QPoint &pos, QGraphicsScene *scene)
    :QGraphicsItemGroup(),
     locked(false),
     borderVisible(true)
{
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges,true); 
    setAcceptHoverEvents(true);
    setPos(pos);

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();

    m_colCurrent = m_colNormal;
    m_pen.setColor(m_colCurrent);

    hGrp = App::GetApplication().GetUserParameter().GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing");
    std::string fontName = hGrp->GetASCII("LabelFont", "osifont");
    m_font.setFamily(QString::fromStdString(fontName));
    m_font.setPointSize(5.f);

    //Add object to scene
    scene->addItem(this);
}

QGraphicsItemView::~QGraphicsItemView()
{

}

void QGraphicsItemView::alignTo(QGraphicsItem *item, const QString &alignment)
{
    alignHash.clear();
    alignHash.insert(alignment, item);
}

QVariant QGraphicsItemView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if(change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();

        if(locked){
            newPos.setX(pos().x());
            newPos.setY(pos().y());
        }

        // TODO  find a better data structure for this
        if(alignHash.size() == 1) {
            QGraphicsItem *item = alignHash.begin().value();
            QString alignMode   = alignHash.begin().key();

            if(alignMode == QString::fromAscii("Vertical")) {
                newPos.setX(item->pos().x());
            } else if(alignMode == QString::fromAscii("Horizontal")) {
                newPos.setY(item->pos().y());
            } else if(alignMode == QString::fromAscii("45slash")) {
                // TODO: This ends up creating some weirdness when the view is
                // moved close to the centre.  Perhaps resolve by creating
                // different constraints for each quadrant rather than two
                // diagonal constraints.
                double relX = newPos.x() - item->pos().x(),
                       relY = newPos.y() - item->pos().y(),
                       dist = ( abs(relX) + abs(relY) ) / 2.0;

                // If we're further into the top-right than bottom-left
                // (remember +Y is down)
                if ( (relX - relY) > (-relX + relY) ) {
                    newPos.setX( item->pos().x() + dist);
                    newPos.setY( item->pos().y() - dist );
                } else {
                    newPos.setX( item->pos().x() - dist );
                    newPos.setY( item->pos().y() + dist );
                }
            } else if(alignMode == QString::fromAscii("45backslash")) {
                double relX = newPos.x() - item->pos().x(),
                       relY = newPos.y() - item->pos().y(),
                       dist = ( abs(relX) + abs(relY) ) / 2.0;

                // If we're further into the top-left than bottom-right
                if ( (-relX - relY) > (relX + relY) ) {
                    newPos.setX( item->pos().x() - dist);
                    newPos.setY( item->pos().y() - dist );
                } else {
                    newPos.setX( item->pos().x() + dist );
                    newPos.setY( item->pos().y() + dist );
                }
            }
        }
        return newPos;
    }

    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            m_colCurrent = m_colSel;
        } else {
            m_colCurrent = m_colNormal;
        }
    }

    return QGraphicsItemGroup::itemChange(change, value);
}

void QGraphicsItemView::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if(this->locked) {
        event->ignore();
    } else {
      QGraphicsItem::mousePressEvent(event);
    }
}

void QGraphicsItemView::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsItem::mouseMoveEvent(event);
}

void QGraphicsItemView::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if(!this->locked && isSelected()) {
        double x = this->x(),
               y = this->getY();
        getViewObject()->X.setValue(x);
        getViewObject()->Y.setValue(y);
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

void QGraphicsItemView::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    // TODO don't like this but only solution at the minute
    if (isSelected()) {
        m_colCurrent = m_colSel;
        return;
    } else {
        m_colCurrent = m_colPre;
        if(this->shape().contains(event->pos())) {                     // TODO don't like this for determining preselect
            m_colCurrent = m_colPre;
        }
    }
    update();
}

void QGraphicsItemView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if(isSelected()) {
        m_colCurrent = m_colSel;
    } else {
        m_colCurrent = m_colNormal;
    }
    update();
}

void QGraphicsItemView::setPosition(qreal x, qreal y)
{
    this->setPos(x, -y);
}

void QGraphicsItemView::updateView(bool update)
{
    if (update ||
        getViewObject()->X.isTouched() ||
        getViewObject()->Y.isTouched()) {
        double featX = getViewObject()->X.getValue();
        double featY = getViewObject()->Y.getValue();
        setPosition(featX,featY);
    }

    if (update ||
        getViewObject()->Rotation.isTouched()) {
        //NOTE: QPainterPaths have to be rotated individually. This transform handles everything else.
        double rot = getViewObject()->Rotation.getValue();
        QPointF centre = boundingRect().center();
        setTransform(QTransform().translate(centre.x(), centre.y()).rotate(-rot).translate(-centre.x(), -centre.y()));
    }

    if (update)
        QGraphicsItem::update();
}

const char * QGraphicsItemView::getViewName() const
{
    return viewName.c_str();
}

Drawing::FeatureView * QGraphicsItemView::getViewObject() const
{
     return viewObj;
}

void QGraphicsItemView::setViewFeature(Drawing::FeatureView *obj)
{
    if(obj == 0)
        return;

    viewObj = obj;
    viewName = obj->getNameInDocument();

    // Set the QGraphicsItemGroup initial position based on the FeatureView
    float x = obj->X.getValue();
    float y = obj->Y.getValue();
    setPosition(x, y);

    Q_EMIT dirty();
}

void QGraphicsItemView::toggleCache(bool state)
{
    this->setCacheMode((state)? NoCache : NoCache);
}

void QGraphicsItemView::drawBorder(QPainter *painter)
{
    painter->save();

    // Make a rectangle smaller than the bounding box as a border and draw dashed line for selection
    QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);

    QPen myPen = m_pen;
    myPen.setStyle(Qt::DashLine);
    myPen.setWidth(0.3);
    painter->setPen(myPen);

    QString name = QString::fromUtf8(this->getViewObject()->Label.getValue());
    painter->setFont(m_font);
    QFontMetrics fm(m_font);
    QPointF pos = box.center();
    pos.setY(box.bottom());
    pos.setX(pos.x() - fm.width(name) / 2.);

    painter->drawText(pos, name);
    painter->drawRect(box);

    painter->restore();
}

void QGraphicsItemView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;
    m_pen.setColor(m_colCurrent);

    if(borderVisible){
         this->drawBorder(painter);
    }
    QGraphicsItemGroup::paint(painter, &myOption, widget);
}

QPainterPath QGraphicsItemView::shape() const {
    QPainterPath path;
    QRectF box = this->boundingRect().adjusted(2.,2.,-2.,-2.);
    path.addRect(box);
    QPainterPathStroker stroker;
    stroker.setWidth(5.f);
    return stroker.createStroke(path);
}

#include "moc_QGraphicsItemView.cpp"

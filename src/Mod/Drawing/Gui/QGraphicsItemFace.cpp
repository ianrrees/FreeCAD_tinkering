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
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainterPathStroker>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#endif

#include <App/Application.h>
#include <App/Material.h>
#include <Base/Console.h>
#include <Base/Parameter.h>

#include "QGraphicsItemView.h"
#include "QGraphicsItemFace.h"

using namespace DrawingGui;

QGraphicsItemFace::QGraphicsItemFace(int ref, QGraphicsScene *scene  ) :
    reference(ref),
    m_fill(Qt::NoBrush)
    //m_fill(Qt::CrossPattern)
{
    if(scene) {
        scene->addItem(this);
    } else {
        Base::Console().Log("PROBLEM? - QGraphicsItemFace(%d) has NO scene\n",ref);
    }
    this->setAcceptHoverEvents(true);

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
        .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing/Colors");
    App::Color fcColor = App::Color((uint32_t) hGrp->GetUnsigned("NormalColor", 0x00000000));
    m_colNormal = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("SelectColor", 0x0000FF00));
    m_colSel = fcColor.asQColor();
    fcColor.setPackedValue(hGrp->GetUnsigned("PreSelectColor", 0x00080800));
    m_colPre = fcColor.asQColor();

    m_brush.setStyle(m_fill);
    setPrettyNormal();
}

QVariant QGraphicsItemFace::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged && scene()) {
        if(isSelected()) {
            setPrettySel();
        } else {
            setPrettyNormal();
        }
        update();
    }
    return QGraphicsItem::itemChange(change, value);
}

void QGraphicsItemFace::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setPrettyPre();
    update();
}

void QGraphicsItemFace::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItemView *view = dynamic_cast<QGraphicsItemView *> (this->parentItem());

    if(!isSelected() && !view->isSelected()) {
        setPrettyNormal();
        update();
    }
}

void QGraphicsItemFace::setPrettyNormal() {
    m_pen.setColor(m_colNormal);
    m_brush.setColor(m_colNormal);
    setPen(m_pen);
    setBrush(m_brush);
}

void QGraphicsItemFace::setPrettyPre() {
    m_pen.setColor(m_colPre);
    m_brush.setColor(m_colPre);
    setPen(m_pen);
    setBrush(m_brush);
}

void QGraphicsItemFace::setPrettySel() {
    m_pen.setColor(m_colSel);
    m_brush.setColor(m_colSel);
    setPen(m_pen);
    setBrush(m_brush);
}



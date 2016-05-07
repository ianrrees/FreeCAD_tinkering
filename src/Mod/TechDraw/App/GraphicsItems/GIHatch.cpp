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
    #include <QPainter>
    #include <QStyleOptionGraphicsItem>
    #include <QSvgRenderer>
#endif // #ifndef _PreComp_

#include "App/Application.h"
#include "App/Material.h"
#include "Base/Parameter.h"

#include "GIHatch.h"

using namespace TechDraw;

GIHatch::GIHatch(std::string parentHatch) :
    m_hatch(parentHatch),
    m_lastFill(""),
    m_fill(Qt::NoBrush)
    //m_fill(Qt::Dense3Pattern)
    //m_fill(Qt::CrossPattern)
    //m_fill(Qt::Dense6Pattern)
{
    auto hGrp( App::GetApplication().GetUserParameter().GetGroup("BaseApp")->
               GetGroup("Preferences")->GetGroup("Mod/TechDraw/Colors") );

    App::Color fcColor;
    fcColor.setPackedValue( hGrp->GetUnsigned("NormalColor", 0x00000000) );
    m_colNormal = fcColor.asQColor();

    m_pen.setCosmetic(true);
    m_pen.setWidthF(1.);
    m_pen.setColor(m_colNormal);

    m_brush.setStyle(m_fill);
}

void GIHatch::paint( QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    QStyleOptionGraphicsItem myOption(*option);
    myOption.state &= ~QStyle::State_Selected;

    painter->setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );   //doesn't seem to change much
    setPen(m_pen);
    setBrush(m_brush);
    QGraphicsPathItem::paint (painter, &myOption, widget);
}


void GIHatch::setColor(App::Color c)
{
    m_colNormal = c.asQColor();
}


void GIHatch::setFill(std::string fillSpec)
{
    if (fillSpec.empty()) {
        return;
    }

    if (fillSpec == m_lastFill) {
        return;
    }

    QString qs(QString::fromStdString(fillSpec));
    m_lastFill = fillSpec;
    //QString qs(QString::fromUtf8("../src/Mod/Drawing/patterns/simple.svg"));
    //QString qs(QString::fromUtf8("../src/Mod/Drawing/patterns/square.svg"));
    QSvgRenderer renderer(qs);
    //QBitmap pixMap(64,64);                         //this size is scene units (mm) instead of pixels?
    //QPixmap::fromImage(m_image);
    //QImage(qt_patternForBrush(style, 0), 8, 8, 1, QImage::Format_MonoLSB);
    //QPixmap::scaled(QSize,QTAspectmode,QTTransformmode)
    QBitmap pixMap(renderer.defaultSize());
    pixMap.fill(Qt::white);   //try  Qt::transparent?
    QPainter painter(&pixMap);
    renderer.render(&painter);                                         //svg texture -> bitmap

    m_texture = pixMap;
    m_brush = QBrush(m_texture);
    m_brush.setStyle(Qt::TexturePattern);
    //m_brush = QBrush(Qt::CrossPattern);
    //m_brush = QBrush(Qt::DiagCrossPattern);
}


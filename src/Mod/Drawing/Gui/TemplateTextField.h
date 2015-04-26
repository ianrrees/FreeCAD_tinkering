/***************************************************************************
 *   Copyright (c) Ian Rees                    (ian.rees@gmail.com) 2015   *
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

#ifndef DRAWINGGUI_TEMPLATETEXTFIELD_H
#define DRAWINGGUI_TEMPLATETEXTFIELD_H

//TODO Precompiled header

#include <QGraphicsRectItem>

#include "../App/FeatureTemplate.h"

QT_BEGIN_NAMESPACE
class QGraphicsItem;
QT_END_NAMESPACE

namespace DrawingGui
{
    /// QGraphicsRectItem-derived class for the text fields in title blocks
    /*!
     * This essentially just a way for us to make a rectangular area which
     * will give us a text editing dialog when clicked so that an appropriate
     * Property in the Drawing's template can be modified.
     * Dear English, I'm sorry.
     */
    class DrawingGuiExport TemplateTextField : public QGraphicsRectItem
    {
        public:
            TemplateTextField(QGraphicsItem *parent,
                              Drawing::FeatureTemplate *myTmplte,
                              const std::string &myFieldName);

            ~TemplateTextField();

            /// Returns the field name that this TemplateTextField represents
            std::string fieldName() const { return fieldNameStr; }
        protected:
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
            Drawing::FeatureTemplate *tmplte;
            std::string fieldNameStr;
    };
}   // namespace DrawingGui

#endif // #ifndef DRAWINGGUI_TEMPLATETEXTFIELD_H


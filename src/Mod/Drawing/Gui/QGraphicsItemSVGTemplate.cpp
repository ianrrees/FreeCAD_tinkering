/***************************************************************************
 *   Copyright (c) 2012-2014 Luke Parry <l.parry@warwick.ac.uk>            *
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
#include <QFile>
#include <QFont>
#include <QPen>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <strstream>
#include <boost/regex.hpp>
#endif // #ifndef _PreComp_

#include <App/Application.h>
#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Parameter.h>
#include <Gui/Command.h>

#include <Mod/Drawing/App/Geometry.h>
#include <Mod/Drawing/App/FeatureSVGTemplate.h>

#include "QGraphicsItemSVGTemplate.h"

using namespace DrawingGui;

QGraphicsItemSVGTemplate::QGraphicsItemSVGTemplate(QGraphicsScene *scene)
    : QGraphicsItemTemplate(scene)
{
    m_svgRender = new QSvgRenderer();

    m_svgItem = new QGraphicsSvgItem();
    m_svgItem->setSharedRenderer(m_svgRender);

    m_svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
    m_svgItem->setCacheMode(QGraphicsItem::NoCache);

    addToGroup(m_svgItem);
}

QGraphicsItemSVGTemplate::~QGraphicsItemSVGTemplate()
{
    clearContents();
    delete m_svgRender;
}

QVariant QGraphicsItemSVGTemplate::itemChange(GraphicsItemChange change,
                                              const QVariant &value)
{
    return QGraphicsItemGroup::itemChange(change, value);
}


void QGraphicsItemSVGTemplate::clearContents()
{
    for (std::vector<TemplateTextField *>::iterator it = textFields.begin();
            it != textFields.end(); ++it) {
        delete *it;
    }
    textFields.clear();
}

void QGraphicsItemSVGTemplate::openFile(const QFile &file)
{

}

void QGraphicsItemSVGTemplate::load(const QString &fileName)
{
    clearContents();

    if (fileName.isEmpty()){
        return;
    }

    QFile file(fileName);
    if (!file.exists()) {
        return;
    }
    m_svgRender->load(file.fileName());

    QSize size = m_svgRender->defaultSize();
    //Base::Console().Log("size of svg document <%i,%i>", size.width(), size.height());
    m_svgItem->setSharedRenderer(m_svgRender);

    Drawing::FeatureSVGTemplate *tmplte = getSVGTemplate();


    //std::string temp = tmplte->Template.getValue();
    std::string temp = tmplte->PageResult.getValue();                    //fixes non-drawing of restored template
    if (temp.empty())
        return;

    Base::FileInfo fi(temp);

    // make a temp file for FileIncluded Property
    std::string tempName = tmplte->PageResult.getExchangeTempFile();
    std::ostringstream ofile;
    std::string tempendl = "--endOfLine--";
    std::string line;

    std::ifstream ifile (fi.filePath().c_str());
    while (!ifile.eof())
    {
        std::getline(ifile,line);
        // check if the marker in the template is found
        if(line.find("<!-- DrawingContent -->") == std::string::npos) {
            // if not -  write through
            ofile << line << tempendl;
        }
    }

    std::string outfragment(ofile.str());

    // Find text tags with freecad:editable attribute and their matching tspans
    boost::regex tagRegex("<text([^>]*freecad:editable=[^>]*)>[^<]*<tspan[^>]*>([^<]*)</tspan>");

    // Smaller regexes for parsing matches to tagRegex
    boost::regex editableNameRegex("freecad:editable=\"([\\w-]+)\"");
    boost::regex boxShapeRegex("freecad:boxShape=\"([\\d.]+)[xX]([\\d.]+)\"");
    boost::regex xRegex("x=\"([\\d.-]+)\"");
    boost::regex yRegex("y=\"([\\d.-]+)\"");

    std::string::const_iterator begin, end;
    begin = outfragment.begin();
    end = outfragment.end();
    boost::match_results<std::string::const_iterator> tagMatch, nameMatch, xMatch, yMatch, boxShapeMatch;

    //TODO: Find location of special fields (first/third angle) and make graphics items for them

    // and update the sketch
    while (boost::regex_search(begin, end, tagMatch, tagRegex)) {
        if ( boost::regex_search(tagMatch[1].first, tagMatch[1].second, nameMatch, editableNameRegex) &&
             boost::regex_search(tagMatch[1].first, tagMatch[1].second, xMatch, xRegex) &&
             boost::regex_search(tagMatch[1].first, tagMatch[1].second, yMatch, yRegex) &&
             boost::regex_search(tagMatch[1].first, tagMatch[1].second, boxShapeMatch, boxShapeRegex) ) {

            QString xStr = QString::fromStdString(xMatch[1].str());
            QString yStr = QString::fromStdString(yMatch[1].str());
            QString editableName = QString::fromStdString(nameMatch[1].str());
            QString widthStr = QString::fromStdString(boxShapeMatch[1].str());
            QString heightStr = QString::fromStdString(boxShapeMatch[2].str());

            double x = xStr.toDouble();
            double y = yStr.toDouble();
            double width = widthStr.toDouble();
            double height = heightStr.toDouble();

            TemplateTextField *item = new TemplateTextField(this, tmplte, nameMatch[1].str());
            float pad = 1;
            item->setRect(x - pad, -tmplte->getHeight() + y - height - pad,
                          width + 2 * pad, height + 2 * pad);

            QPen myPen;
            myPen.setStyle(Qt::DashLine);
            myPen.setWidth(0);  // 0 means "cosmetic pen" - always 1px
            item->setPen(myPen);

            item->setZValue(100);
            addToGroup(item);
            textFields.push_back(item);
        }

        begin = tagMatch[0].second;
    }

    double xaspect, yaspect;
    xaspect = tmplte->getWidth() / (double) size.width();
    yaspect = tmplte->getHeight() / (double) size.height();

    QTransform qtrans;
    qtrans.translate(0.f, -tmplte->getHeight());
    qtrans.scale(xaspect , yaspect);
    m_svgItem->setTransform(qtrans);
}

Drawing::FeatureSVGTemplate * QGraphicsItemSVGTemplate::getSVGTemplate()
{
    if(pageTemplate && pageTemplate->isDerivedFrom(Drawing::FeatureSVGTemplate::getClassTypeId()))
        return static_cast<Drawing::FeatureSVGTemplate *>(pageTemplate);
    else
        return 0;
}

void QGraphicsItemSVGTemplate::draw()
{
    Drawing::FeatureSVGTemplate *tmplte = getSVGTemplate();
    if(!tmplte)
        throw Base::Exception("Template Feature not set for QGraphicsItemSVGTemplate");

    load(QString::fromUtf8(tmplte->PageResult.getValue()));
}

void QGraphicsItemSVGTemplate::updateView(bool update)
{
    draw();
}

#include "moc_QGraphicsItemSVGTemplate.cpp"

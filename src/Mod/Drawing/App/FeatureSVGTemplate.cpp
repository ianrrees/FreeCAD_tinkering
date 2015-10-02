/***************************************************************************
 *   Copyright (c) 2014 Luke Parry <l.parry@warwick.ac.uk>                 *
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2002     *
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
  #include <sstream>
  #include <QDomDocument>
  #include <QFile>
#endif

#include <Base/Exception.h>
#include <Base/Console.h>
#include <Base/Interpreter.h>
#include <Base/FileInfo.h>
#include <Base/PyObjectBase.h>
#include <Base/Quantity.h>

#include <App/Application.h>

#include <boost/regex.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <iostream>
#include <iterator>

#include <QDebug>

#include "FeaturePage.h"
#include "FeatureSVGTemplate.h"

#include "FeatureSVGTemplatePy.h"  //?? !exists()

using namespace Drawing;
using namespace std;

PROPERTY_SOURCE(Drawing::FeatureSVGTemplate, Drawing::FeatureTemplate)

FeatureSVGTemplate::FeatureSVGTemplate()
{
    static const char *group = "Drawing view";

    //TODO: Do we need PageResult anymore?
    ADD_PROPERTY_TYPE(PageResult, (0),  group, App::Prop_Output,    "Resulting SVG document of that page");
    ADD_PROPERTY_TYPE(Template,   (""), group, App::Prop_Transient, "Template for the page");

    // Width and Height properties shouldn't be set by the user
    Height.StatusBits.set(2);       // Read Only
    Width.StatusBits.set(2);       // Read Only
    Orientation.StatusBits.set(2); // Read Only
}

FeatureSVGTemplate::~FeatureSVGTemplate()
{
}

/*
std::string FeatureSVGTemplate::getSvgIdForEditable(const std::string &editableName)
{
    if (editableSvgIds.count(editableName)) {
        return editableSvgIds[editableName];
    } else {
        return "";
    }
}*/

PyObject *FeatureSVGTemplate::getPyObject(void)
{
    if (PythonObject.is(Py::_None())) {
        // ref counter is set to 1
        PythonObject = Py::Object(new FeatureSVGTemplatePy(this),true);
    }
    return Py::new_reference_to(PythonObject);
}

unsigned int FeatureSVGTemplate::getMemSize(void) const
{
    return 0;
}

short FeatureSVGTemplate::mustExecute() const
{
    return Drawing::FeatureTemplate::mustExecute();
}

/// get called by the container when a Property was changed
void FeatureSVGTemplate::onChanged(const App::Property* prop)
{
    bool updatePage = false;

    if (prop == &PageResult) {
        if (isRestoring()) {

            // When loading a document the included file
            // doesn't need to exist at this point.
            Base::FileInfo fi(PageResult.getValue());

//TODO: I don't understand why it is that this "works", seems like we might have two properties where we only need one?  -Ian-
Template.setValue(App::Application::getResourceDir() + "Mod/Drawing/Templates/" + fi.fileName());

            if (!fi.exists()) {
                return;
            }
        }
    } else if (prop == &Template) {
        if (!isRestoring()) {
            EditableTexts.setValues(getEditableTextsFromTemplate());
            updatePage = true;
        }
    } else if (prop == &EditableTexts) {
        if (!isRestoring()) {
            updatePage = true;
        }
    }

    if (updatePage) {
        execute();

        // Update the parent page if exists
        std::vector<App::DocumentObject*> parent = getInList();
        for (std::vector<App::DocumentObject*>::iterator it = parent.begin(); it != parent.end(); ++it) {
            if ((*it)->getTypeId().isDerivedFrom(FeaturePage::getClassTypeId())) {
                Drawing::FeaturePage *page = static_cast<Drawing::FeaturePage *>(*it);
                page->touch();
            }
        }
    }

    Drawing::FeatureTemplate::onChanged(prop);
}

App::DocumentObjectExecReturn * FeatureSVGTemplate::execute(void)
{
    std::string temp = Template.getValue();
    if (temp.empty())
        return App::DocumentObject::StdReturn;

    Base::FileInfo fi(temp);
    if (!fi.isReadable()) {
        // if there is a old absolute template file set use a redirect
        fi.setFile(App::Application::getResourceDir() + "Mod/Drawing/Templates/" + fi.fileName());
        // try the redirect
        if (!fi.isReadable()) {
            Base::Console().Log("FeaturePage::execute() not able to open %s!\n",Template.getValue());
            std::string error = std::string("Cannot open file ") + Template.getValue();
            return new App::DocumentObjectExecReturn(error);
        }
    }

    if (std::string(PageResult.getValue()).empty())
        PageResult.setValue(fi.filePath().c_str());

    // open Template file
    string line;
    ifstream file (fi.filePath().c_str());

    // make a temp file for FileIncluded Property
    string tempName = PageResult.getExchangeTempFile();
    ostringstream ofile;
    string tempendl = "--endOfLine--";

    while (!file.eof())
    {
        getline(file,line);
        // check if the marker in the template is found
        if(line.find("<!-- DrawingContent -->") == string::npos) {
            // if not -  write through
            ofile << line << tempendl;
        }

        //double t0, t1,t2,t3;
        float t0, t1,t2,t3;
        if(line.find("<!-- Title block") != std::string::npos) {
            sscanf(line.c_str(), "%*s %*s %*s %f %f %f %f", &t0, &t1, &t2, &t3);    //eg "    <!-- Working space 10 10 410 287 -->"
            blockDimensions = QRectF(t0, t1, t2 - t0, t3 - t1);
        }

    }
    file.close();

    // checking for freecad editable texts
    string outfragment(ofile.str());

    std::map<std::string, std::string> subs = EditableTexts.getValues();

    if (subs.size() > 0) {
        boost::regex e1 ("<text.*?freecad:editable=\"(.*?)\".*?<tspan.*?>(.*?)</tspan>");
        string::const_iterator begin, end;
        begin = outfragment.begin();
        end = outfragment.end();
        boost::match_results<std::string::const_iterator> what;

        // Find editable texts
        while (boost::regex_search(begin, end, what, e1)) {
            // if we have a replacement value for the text we've found
            if (subs.count(what[1].str())) {
                 // change it to specified value
                 boost::regex e2 ("(<text.*?freecad:editable=\"" + what[1].str() + "\".*?<tspan.*?)>(.*?)(</tspan>)");
                 outfragment = boost::regex_replace(outfragment, e2, "$1>" + subs[what[1].str()] + "$3");
            }
            begin = what[0].second;
        }
    }


    // restoring linebreaks and saving the file
    boost::regex e3 ("--endOfLine--");
    string fmt = "\\n";
    outfragment = boost::regex_replace(outfragment, e3, fmt);
    ofstream outfinal(tempName.c_str());
    outfinal << outfragment;
    outfinal.close();

    PageResult.setValue(tempName.c_str());


    // Calculate the dimensions of the page and store for retrieval

    QFile resultFile(QString::fromAscii(PageResult.getValue()));
    if (!resultFile.exists()) {
        throw Base::Exception("Couldn't load document from PageResult");
    }

    QDomDocument doc(QString::fromAscii("mydocument"));

    if (!doc.setContent(&resultFile)) {
        resultFile.close();
        throw Base::Exception("Couldn't parse template SVG contents");
    }

    // Parse the document XML
    QDomElement docElem = doc.documentElement();

    // Obtain the size of the SVG document by reading the document attirbutes
    Base::Quantity quantity;

    // Obtain the width
    QString str = docElem.attribute(QString::fromAscii("width"));
    quantity = Base::Quantity::parse(str);
    quantity.setUnit(Base::Unit::Length);

    Width.setValue(quantity.getValue());

    str = docElem.attribute(QString::fromAscii("height"));
    quantity = Base::Quantity::parse(str);
    quantity.setUnit(Base::Unit::Length);

    Height.setValue(quantity.getValue());

    bool isLandscape = getWidth() / getHeight() >= 1.;

    Orientation.setValue(isLandscape ? 1 : 0);

    // Housekeeping close the file
    resultFile.close();

    touch();

    return Drawing::FeatureTemplate::execute();
}

void FeatureSVGTemplate::getBlockDimensions(double &x, double &y, double &width, double &height) const
{
    x      = blockDimensions.left();
    y      = blockDimensions.bottom();
    width  = blockDimensions.width();
    height = blockDimensions.height();
}

double FeatureSVGTemplate::getWidth() const
{
    return Width.getValue();
}

double FeatureSVGTemplate::getHeight() const
{
    return Height.getValue();
}


std::map<std::string, std::string> FeatureSVGTemplate::getEditableTextsFromTemplate()
{
    std::map<std::string, std::string> eds;

    std::string temp = Template.getValue();
    if (!temp.empty()) {
        Base::FileInfo tfi(temp);
        if (!tfi.isReadable()) {
            // if there is a old absolute template file set use a redirect
            tfi.setFile(App::Application::getResourceDir() + "Mod/Drawing/Templates/" + tfi.fileName());
            // try the redirect
            if (!tfi.isReadable()) {
                return eds;
            }
        }
        string tline, tfrag;
        ifstream tfile (tfi.filePath().c_str());
        while (!tfile.eof()) {
            getline (tfile,tline);
            tfrag += tline;
            tfrag += "--endOfLine--";
        }
        tfile.close();
        boost::regex e ("<text.*?id=\"(.*?)\".*?freecad:editable=\"(.*?)\".*?<tspan.*?>(.*?)</tspan>");
        string::const_iterator tbegin, tend;
        tbegin = tfrag.begin();
        tend = tfrag.end();
        boost::match_results<std::string::const_iterator> twhat;
        //TODO: "overly complex regex" error in this while loop for some templates (ex: ../ISO/A/ISO_A4_Landscape.svg)
        //      (old)Draw1::FeaturePage.cpp code doesn't fail
        while (boost::regex_search(tbegin, tend, twhat, e)) {
/*            string temp1 = twhat[1],
                   temp2 = twhat[2],
                   temp3 = twhat[3];
            qDebug()<<"Got field id:"<<temp1.c_str()<<" editable as:"<<temp2.c_str()<<" default text:"<<temp3.c_str()<<".";
*/
            if (eds.count(twhat[2]) > 0) {
                //TODO: Throw or [better] change key
                qDebug() << "Got duplicate value for key "<<((string)twhat[3]).c_str();
            } else {
                //TODO: May also need to maintain an internal map to get SVG ID
                eds[twhat[2]] = twhat[3];
                //editableSvgIds[twhat[2]] = twhat[1];
            }

            tbegin = twhat[0].second;
        }
    }
    return eds;
}

// Python Template feature ---------------------------------------------------------
namespace App {
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(Drawing::FeatureSVGTemplatePython, Drawing::FeatureSVGTemplate)
template<> const char* Drawing::FeatureSVGTemplatePython::getViewProviderName(void) const {
    return "DrawingGui::ViewProviderPython";
}
/// @endcond

// explicit template instantiation
template class DrawingExport FeaturePythonT<Drawing::FeatureSVGTemplate>;
}

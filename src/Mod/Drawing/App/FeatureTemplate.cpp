/***************************************************************************
 *   Copyright (c) 2014 Luke Parry <l.parry@warwick.ac.uk>                 *
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
# include <sstream>
#endif

#include <Base/Exception.h>
#include <Base/Console.h>
#include <Base/Interpreter.h>
#include <Base/FileInfo.h>

#include <App/Application.h>

#include <iostream>
#include <iterator>

#include "FeaturePage.h"
#include "FeatureTemplate.h"
#include "FeatureTemplatePy.h"

using namespace Drawing;
using namespace std;

PROPERTY_SOURCE(Drawing::FeatureTemplate, App::DocumentObject)


const char* FeatureTemplate::OrientationEnums[]= {"Portrait",
                                                  "Landscape",
                                                  NULL};



FeatureTemplate::FeatureTemplate(void)
{
    const char *group = "Page Properties";

    Orientation.setEnums(OrientationEnums);
    ADD_PROPERTY(Orientation, ((long)0));

    // Physical Properties inherent to every template class
    ADD_PROPERTY_TYPE(Width,     (0),  group, (App::PropertyType)(App::Prop_None), "Width of page");
    ADD_PROPERTY_TYPE(Height,    (0),  group, (App::PropertyType)(App::Prop_None), "Height of page");
    ADD_PROPERTY_TYPE(PaperSize, (""), group, (App::PropertyType)(App::Prop_None), "Paper Format");

    ADD_PROPERTY_TYPE(EditableTexts, (), group, (App::PropertyType)(App::Prop_None),
                      "Editable strings in the template");
}

FeatureTemplate::~FeatureTemplate()
{
  Base::Console().Log("template destroyed");
}


PyObject *FeatureTemplate::getPyObject(void)
{
    if (PythonObject.is(Py::_None())) {
        // ref counter is set to 1
        PythonObject = Py::Object(new FeatureTemplatePy(this),true);
    }
    return Py::new_reference_to(PythonObject);
}

unsigned int FeatureTemplate::getMemSize(void) const
{
    return 0;
}

double FeatureTemplate::getWidth() const
{
    return Width.getValue();
}

double FeatureTemplate::getHeight() const
{
    return Height.getValue();
}

short FeatureTemplate::mustExecute() const
{
    return App::DocumentObject::mustExecute();
}

/// get called by the container when a Property was changed
void FeatureTemplate::onChanged(const App::Property* prop)
{
    App::DocumentObject::onChanged(prop);
}

App::DocumentObjectExecReturn *FeatureTemplate::execute(void)
{
    FeaturePage *page = 0;
    std::vector<App::DocumentObject*> parent = getInList();
    for (std::vector<App::DocumentObject*>::iterator it = parent.begin(); it != parent.end(); ++it) {
        if ((*it)->getTypeId().isDerivedFrom(FeaturePage::getClassTypeId())) {
            page = dynamic_cast<Drawing::FeaturePage *>(*it);
        }
    }

    if(page) {
        page->Template.touch();
    }

    return App::DocumentObject::execute();
}

void FeatureTemplate::getBlockDimensions(double &x, double &y, double &width, double &height) const
{
    throw Base::Exception("implement in virtual function");
}

// Python Template feature ---------------------------------------------------------

namespace App {
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(Drawing::FeatureTemplatePython, Drawing::FeatureTemplate)
template<> const char* Drawing::FeatureTemplatePython::getViewProviderName(void) const {
    return "DrawingGui::ViewProviderPython";
}
/// @endcond

// explicit template instantiation
template class DrawingExport FeaturePythonT<Drawing::FeatureTemplate>;
}   // namespace App

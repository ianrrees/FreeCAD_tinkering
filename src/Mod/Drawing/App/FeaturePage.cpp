/***************************************************************************
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
# include <sstream>
# include <iostream>
# include <iterator>
#endif

#include <Base/Exception.h>
#include <Base/Console.h>
#include <Base/FileInfo.h>
#include <App/Application.h>
#include <App/Document.h>
#include <boost/regex.hpp>

#include "FeaturePage.h"
#include "FeatureView.h"
#include "FeatureProjGroup.h"
#include "FeatureViewClip.h"
#include "FeatureTemplate.h"
#include "FeatureViewCollection.h"
#include "FeatureViewPart.h"
#include "FeatureViewDimension.h"

#include "FeaturePagePy.h"  // generated from FeaturePagePy.xml

using namespace Drawing;
using namespace std;


//===========================================================================
// FeaturePage
//===========================================================================

PROPERTY_SOURCE(Drawing::FeaturePage, App::DocumentObject)

const char* FeaturePage::ProjectionTypeEnums[]= {"First Angle",
                                                 "Third Angle",
                                                 NULL};

FeaturePage::FeaturePage(void)
{
    static const char *group = "Page";

    ADD_PROPERTY_TYPE(Template, (0), group, (App::PropertyType)(App::Prop_None), "Attached Template");
    ADD_PROPERTY_TYPE(Views, (0), group, (App::PropertyType)(App::Prop_None),"Attached Views");

    // Projection Properties
    ProjectionType.setEnums(ProjectionTypeEnums);
    ADD_PROPERTY(ProjectionType, ((long)0));
    ADD_PROPERTY_TYPE(Scale ,(1.0), group, App::Prop_None, "Scale factor for this Page");
}

FeaturePage::~FeaturePage()
{
}

void FeaturePage::onBeforeChange(const App::Property* prop)
{
    App::DocumentObject::onBeforeChange(prop);
}

void FeaturePage::onChanged(const App::Property* prop)
{
    if (prop == &Template) {
        if (!isRestoring()) {
        //TODO: reload if Template prop changes (ie different Template)
        Base::Console().Message("TODO: Unimplemented function FeaturePage::onChanged(Template)\n");
        }
    } else if (prop == &Views) {
        if (!isRestoring()) {
            //TODO: reload if Views prop changes (ie adds/deletes)
            //touch();
        }
    } else if(prop == &Scale) {
        // touch all views in the Page as they may be dependent on this scale
      const std::vector<App::DocumentObject*> &vals = Views.getValues();
      for(std::vector<App::DocumentObject *>::const_iterator it = vals.begin(); it < vals.end(); ++it) {
          Drawing::FeatureView *view = dynamic_cast<Drawing::FeatureView *>(*it);
          if (view != NULL && view->ScaleType.isValue("Document")) {
              view->Scale.touch();
          }
      }
    } else if (prop == &ProjectionType) {
      // touch all ortho views in the Page as they may be dependent on Projection Type
      const std::vector<App::DocumentObject*> &vals = Views.getValues();
      for(std::vector<App::DocumentObject *>::const_iterator it = vals.begin(); it < vals.end(); ++it) {
          Drawing::FeatureProjGroup *view = dynamic_cast<Drawing::FeatureProjGroup *>(*it);
          if (view != NULL && view->ProjectionType.isValue("Document")) {
              view->ProjectionType.touch();
          }
      }

      // TODO: Also update Template graphic.

    }
    App::DocumentObject::onChanged(prop);
}

App::DocumentObjectExecReturn *FeaturePage::execute(void)
{
    Template.touch();
    Views.touch();
    return App::DocumentObject::StdReturn;
}

short FeaturePage::mustExecute() const
{
    if(Scale.isTouched())
        return 1;

    // Check the value of template if this has been modified
    App::DocumentObject* tmpl = Template.getValue();
    if(tmpl && tmpl->isTouched())
        return 1;

    // Check if within this Page, any Views have been touched
    bool ViewsTouched = false;
    const std::vector<App::DocumentObject*> &vals = Views.getValues();
    for(std::vector<App::DocumentObject *>::const_iterator it = vals.begin(); it < vals.end(); ++it) {
       if((*it)->isTouched()) {
            return 1;
        }
    }

    return (ViewsTouched) ? 1 : App::DocumentObject::mustExecute();
}

PyObject *FeaturePage::getPyObject(void)
{
    if (PythonObject.is(Py::_None())){
        // ref counter is set to 1
        PythonObject = Py::Object(new FeaturePagePy(this),true);
    }

    return Py::new_reference_to(PythonObject); 
}

bool FeaturePage::hasValidTemplate() const
{
    App::DocumentObject *obj = 0;
    obj = Template.getValue();

    if(obj && obj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId())) {
        Drawing::FeatureTemplate *templ = static_cast<Drawing::FeatureTemplate *>(obj);
        if (templ->getWidth() > 0. &&
            templ->getHeight() > 0.) {
            return true;
        }
    }

    return false;
}

double FeaturePage::getPageWidth() const
{
    App::DocumentObject *obj = 0;
    obj = Template.getValue();

    if( obj && obj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId()) ) {
        Drawing::FeatureTemplate *templ = static_cast<Drawing::FeatureTemplate *>(obj);
        return templ->getWidth();
    }

    throw Base::Exception("Template not set for Page");
}

double FeaturePage::getPageHeight() const
{
    App::DocumentObject *obj = 0;
    obj = Template.getValue();

    if(obj) {
        if(obj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId())) {
            Drawing::FeatureTemplate *templ = static_cast<Drawing::FeatureTemplate *>(obj);
            return templ->getHeight();
        }
    }

    throw Base::Exception("Template not set for Page");
}

const char * FeaturePage::getPageOrientation() const
{
    App::DocumentObject *obj;
    obj = Template.getValue();

    if(obj) {
        if(obj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId())) {
          Drawing::FeatureTemplate *templ = static_cast<Drawing::FeatureTemplate *>(obj);

          return templ->Orientation.getValueAsString();
        }
    }
    throw Base::Exception("Template not set for Page");
}

int FeaturePage::addView(App::DocumentObject *docObj)
{
    if(!docObj->isDerivedFrom(Drawing::FeatureView::getClassTypeId()))
        return -1;

    const std::vector<App::DocumentObject *> currViews = Views.getValues();
    std::vector<App::DocumentObject *> newViews(currViews);
    newViews.push_back(docObj);
    Views.setValues(newViews);
    Views.touch();
    return Views.getSize();
}

int FeaturePage::removeView(App::DocumentObject *docObj)
{
    if(!docObj->isDerivedFrom(Drawing::FeatureView::getClassTypeId()))
        return -1;

    const std::vector<App::DocumentObject*> currViews = Views.getValues();
    std::vector<App::DocumentObject*> newViews;
    std::vector<App::DocumentObject*>::const_iterator it = currViews.begin();
    for (; it != currViews.end(); it++) {
        std::string viewName = docObj->getNameInDocument();
        if (viewName.compare((*it)->getNameInDocument()) != 0) {
            newViews.push_back((*it));
        }
    }
    Views.setValues(newViews);
    Views.touch();

    return Views.getSize();
}

void FeaturePage::onDocumentRestored()
{
    std::vector<App::DocumentObject*> featViews = Views.getValues();
    std::vector<App::DocumentObject*>::const_iterator it = featViews.begin();
    //first, make sure all the Parts have been executed so GeometryObjects exist
    for(; it != featViews.end(); ++it) {
        Drawing::FeatureViewPart *part = dynamic_cast<Drawing::FeatureViewPart *>(*it);
        if (part != NULL) {
            part->execute();
        }
    }
    //second, make sure all the Dimensions have been executed so Measurements have References
    for(it = featViews.begin(); it != featViews.end(); ++it) {
        Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(*it);
        if (dim != NULL) {
            dim->execute();
        }
    }
    recompute();
    App::DocumentObject::onDocumentRestored();
}


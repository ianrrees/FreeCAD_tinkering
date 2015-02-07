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
#endif


#include <Base/Exception.h>
#include <Base/Console.h>
#include <Base/FileInfo.h>
#include <App/Application.h>
#include <App/Document.h>
#include <boost/regex.hpp>
#include <iostream>
#include <iterator>

#include "FeaturePage.h"
#include "FeatureView.h"
#include "FeatureClip.h"
#include "FeatureTemplate.h"
#include "FeatureViewCollection.h"

using namespace Drawing;
using namespace std;


//===========================================================================
// FeaturePage
//===========================================================================

PROPERTY_SOURCE(Drawing::FeaturePage, App::DocumentObjectGroup)

const char *group = "Drawing view";

const char* FeaturePage::OrthoProjectionTypeEnums[]= {"First Angle",
                                                      "Third Angle",
                                                      NULL};

FeaturePage::FeaturePage(void) : numChildren(0)
{
    static const char *group = "Page";

//obs    ADD_PROPERTY_TYPE(PageResult ,(0),group,App::Prop_Output,"Resulting SVG document of that page");
    ADD_PROPERTY_TYPE(Template ,(0), group, (App::PropertyType)(App::Prop_Transient),"Attached Template");
//obs    ADD_PROPERTY_TYPE(EditableTexts,(""),group,App::Prop_None,"Substitution values for the editable strings in the template");
    ADD_PROPERTY_TYPE(Views    ,(0), group, (App::PropertyType)(App::Prop_None),"Attached Views");

    // Projection Properties
    OrthoProjectionType.setEnums(OrthoProjectionTypeEnums);
    ADD_PROPERTY(OrthoProjectionType,((long)0));
    ADD_PROPERTY_TYPE(Scale ,(1.0)     ,group,App::Prop_None,"Scale factor for this Page");
}

FeaturePage::~FeaturePage()
{
}

void FeaturePage::onBeforeChange(const App::Property* prop)
{
    if (prop == &Group) {
        numChildren = Group.getSize();
    }

    App::DocumentObjectGroup::onBeforeChange(prop);
}

/// get called by the container when a Property was changed
void FeaturePage::onChanged(const App::Property* prop)
{
    if (prop == &Template) {
        if (!this->isRestoring()) {
        //TODO: reload if Template prop changes (ie different Template)
        Base::Console().Message("TODO: Unimplemented function FeaturePage::onChanged(Template)\n");
        }
    }

    if (prop == &Views) {
        if (!this->isRestoring()) {
            //TODO: reload if Views prop changes (ie adds/deletes)
            //this->touch();
        }
    }

    if (prop == &Group) {
        if (Group.getSize() != numChildren) {
            numChildren = Group.getSize();
            touch();
        }
    }

    if(prop == &Scale) {
        // touch all views in the Page as they may be dependent on this scale
      const std::vector<App::DocumentObject*> &vals = Views.getValues();
      for(std::vector<App::DocumentObject *>::const_iterator it = vals.begin(); it < vals.end(); ++it) {
          Drawing::FeatureView *view = dynamic_cast<Drawing::FeatureView *>(*it);
          if(strcmp(view->ScaleType.getValueAsString(), "Document") == 0) {
              view->Scale.touch();
          }
      }
    }
    App::DocumentObjectGroup::onChanged(prop);
}

App::DocumentObjectExecReturn *FeaturePage::execute(void)
{
    Template.touch();
    Views.touch();
    return App::DocumentObject::StdReturn;
}

void FeaturePage::onBeforeChange(const App::Property* prop)
{
    if (prop == &Group) {
        numChildren = Group.getSize();
    }

    App::DocumentObjectGroup::onBeforeChange(prop);
}

short FeaturePage::mustExecute() const
{
    // If Tolerance Property is touched
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

    return (ViewsTouched) ? 1 : App::DocumentObjectGroup::mustExecute();
}
bool FeaturePage::hasValidTemplate() const
{
    App::DocumentObject *obj = 0;
    obj = Template.getValue();

    if(obj && obj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId())) {
        Drawing::FeatureTemplate *templ = static_cast<Drawing::FeatureTemplate *>(obj);
        if(templ->getWidth() > 0. &&
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

    if(obj) {
        if(obj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId()) ) {
        Drawing::FeatureTemplate *templ = static_cast<Drawing::FeatureTemplate *>(obj);
        return templ->getWidth();
        }
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


}

void FeaturePage::onDocumentRestored()
{
    // Needs to be tmp. set because otherwise the custom text gets overridden (#0002064)
    this->StatusBits.set(4); // the 'Restore' flag

    Base::FileInfo fi(PageResult.getValue());
    std::string path = App::Application::getResourceDir() + "Mod/Drawing/Templates/" + fi.fileName();
    // try to find the template in user dir/Templates first
    Base::FileInfo tempfi(App::Application::getUserAppDataDir() + "Templates/" + fi.fileName());
    if (tempfi.exists())
        path = tempfi.filePath();
    Template.setValue(path);

    this->StatusBits.reset(4); // the 'Restore' flag

int FeaturePage::addView(App::DocumentObject *docObj)
{
    if(!docObj->isDerivedFrom(Drawing::FeatureView::getClassTypeId()))
        return -1; //Doc Object must be derived from a Part Feature

    const std::vector<App::DocumentObject *> vals = Views.getValues();
    std::vector<App::DocumentObject *> newVals(vals);
    newVals.push_back(docObj);
    Views.setValues(newVals);

    if(docObj->getTypeId().isDerivedFrom(Drawing::FeatureViewCollection::getClassTypeId())) {
        // Add child views recursively to the page feature
        Drawing::FeatureViewCollection *collection = dynamic_cast<Drawing::FeatureViewCollection *>(docObj);
        const std::vector<App::DocumentObject *> & views = collection->Views.getValues();
        for(std::vector<App::DocumentObject*>::const_iterator it = views.begin(); it != views.end(); ++it) {
            //this->addView(*it); // Recursively add child views
        }
    }

    Views.touch();
    return Views.getSize();
}



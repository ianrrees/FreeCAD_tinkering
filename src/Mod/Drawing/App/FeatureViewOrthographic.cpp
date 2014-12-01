/***************************************************************************
 *   Copyright (c) 2013-2014 Luke Parry <l.parry@warwick.ac.uk>            *
 *   Copyright (c) 2014 Joe Dowsett <dowsettjoe[at]yahoo[dot]co[dot]uk>    *
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

#include <App/Document.h>
#include <App/DocumentObject.h>

#include <Base/BoundBox.h>
#include <Base/Console.h>
#include <Base/Exception.h>

#include <QRectF>

#include <Mod/Part/App/PartFeature.h>
#include "FeaturePage.h"
#include "FeatureOrthoView.h"
#include "FeatureViewOrthographic.h"

using namespace Drawing;

const char* FeatureViewOrthographic::ProjectionTypeEnums[]= {"Document",
                                                             "First Angle",
                                                             "Third Angle",
                                                             NULL};


PROPERTY_SOURCE(Drawing::FeatureViewOrthographic, Drawing::FeatureViewCollection)

FeatureViewOrthographic::FeatureViewOrthographic(void)
{
    static const char *group = "Drawing view";

    ADD_PROPERTY_TYPE(Anchor    ,(0), group, App::Prop_None,"The root view to align projections with");

    ProjectionType.setEnums(ProjectionTypeEnums);
    ADD_PROPERTY(ProjectionType,((long)0));
}

short FeatureViewOrthographic::mustExecute() const
{
    if(Views.isTouched() ||
       Source.isTouched()) {
        return 1;
     }

    if (ProjectionType.isTouched())
        return 1;
    return Drawing::FeatureViewCollection::mustExecute();
}

Base::BoundBox3d FeatureViewOrthographic::getBoundingBox() const
{
    Base::BoundBox3d bbox;

    std::vector<App::DocumentObject*> views = Views.getValues();
    for (std::vector<App::DocumentObject*>::const_iterator it = views.begin(); it != views.end(); ++it) {
         if ((*it)->getTypeId().isDerivedFrom(FeatureViewPart::getClassTypeId())) {
            FeatureViewPart *part = static_cast<FeatureViewPart *>(*it);
            Base::BoundBox3d  bb = part->getBoundingBox();

            bb.ScaleX(1. / part->Scale.getValue());
            bb.ScaleY(1. / part->Scale.getValue());

            bb.MoveX(part->X.getValue());
            bb.MoveY(part->Y.getValue());
            
            bbox.Add(bb);

        }
    }
    return bbox;
}

// Function provided by Joe Dowsett, 2014
double FeatureViewOrthographic::calculateAutomaticScale() const
{

    Drawing::FeaturePage *page = 0;

    std::vector<App::DocumentObject*> parent = getInList();
    for (std::vector<App::DocumentObject*>::iterator it = parent.begin(); it != parent.end(); ++it) {
        if ((*it)->getTypeId().isDerivedFrom(FeaturePage::getClassTypeId())) {
            page = static_cast<FeaturePage *>(*it);
        }
    }

    if(!page)
      throw Base::Exception("No page is assigned to this feature");


    if(!page->hasValidTemplate())
        throw Base::Exception("Page template isn't valid");

    Base::BoundBox3d bbox = this->getBoundingBox();

    float scale_x = (page->getPageWidth()) / bbox.LengthX(); // TODO include gap spaces
    float scale_y = (page->getPageHeight()) / bbox.LengthY();

    float working_scale = std::min(scale_x, scale_y);

    //which gives the largest scale for which the min_space requirements can be met, but we want a 'sensible' scale, rather than 0.28457239...
    //eg if working_scale = 0.115, then we want to use 0.1, similarly 7.65 -> 5, and 76.5 -> 50

    float exponent = std::floor(std::log10(working_scale));                  //if working_scale = a * 10^b, what is b?
    working_scale *= std::pow(10, -exponent);                                //now find what 'a' is.

    float valid_scales[2][8] = {{1.0, 1.25, 2.0, 2.5, 3.75, 5.0, 7.5, 10.0},   //equate to 1:10, 1:8, 1:5, 1:4, 3:8, 1:2, 3:4, 1:1
                                {1.0, 1.5 , 2.0, 3.0, 4.0 , 5.0, 8.0, 10.0}};  //equate to 1:1, 3:2, 2:1, 3:1, 4:1, 5:1, 8:1, 10:1

    int i = 7;
    while (valid_scales[(exponent >= 0)][i] > working_scale)                 //choose closest value smaller than 'a' from list.
        i -= 1;                                                              //choosing top list if exponent -ve, bottom list for +ve exponent

    return valid_scales[(exponent >= 0)][i] * pow(10, exponent);            //now have the appropriate scale, reapply the *10^b
}

/// get called by the container when a Property was changed
void FeatureViewOrthographic::onChanged(const App::Property* prop)
{
    Drawing::FeatureViewCollection::onChanged(prop);

    if (prop == &ProjectionType ||
        prop == &ScaleType ||
        prop == &Scale && !Scale.StatusBits.test(5)){
          if (!this->isRestoring()) {
              this->execute();
          }
    }

}

FeatureViewOrthographic::~FeatureViewOrthographic()
{
}

App::DocumentObject * FeatureViewOrthographic::getOrthoView(const char *viewProjType) const
{
    const std::vector<App::DocumentObject *> &views = Views.getValues();
    for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {

        Drawing::FeatureView *view = dynamic_cast<Drawing::FeatureView *>(*it);
        if(view->getTypeId() == Drawing::FeatureOrthoView::getClassTypeId()) {
            Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(*it);

            if(strcmp(viewProjType, orthoView->Type.getValueAsString()) == 0)
                return *it;
        }
    }

    return 0;
}

bool FeatureViewOrthographic::hasOrthoView(const char *viewProjType) const
{
    const std::vector<App::DocumentObject *> &views = Views.getValues();

    for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {

        Drawing::FeatureView *view = dynamic_cast<Drawing::FeatureView *>(*it);
        if(view->getTypeId() == Drawing::FeatureOrthoView::getClassTypeId()) {
            Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(*it);

            if(strcmp(viewProjType, orthoView->Type.getValueAsString()) == 0)
                return true;
        }
    }
    return false;
}

App::DocumentObject * FeatureViewOrthographic::addOrthoView(const char *viewProjType)
{
    // Find a more elegant way of validating the type
    if(strcmp(viewProjType, "Front")  == 0 ||
       strcmp(viewProjType, "Left")   == 0 ||
       strcmp(viewProjType, "Right")  == 0 ||
       strcmp(viewProjType, "Top")    == 0 ||
       strcmp(viewProjType, "Bottom") == 0 ||
       strcmp(viewProjType, "Rear")   == 0 ) {

        if(hasOrthoView(viewProjType)) {
            throw Base::Exception("The Projection is already used in this group");
        }

        std::string FeatName = this->getDocument()->getUniqueObjectName("OrthoView");
        App::DocumentObject *docObj = this->getDocument()->addObject("Drawing::FeatureOrthoView",
                                                                     FeatName.c_str());
        Drawing::FeatureOrthoView *view = dynamic_cast<Drawing::FeatureOrthoView *>(docObj);
        view->Source.setValue(this->Source.getValue());
        //view->Scale.setValue(this->Scale.getValue());
        view->Type.setValue(viewProjType);

        std::string label = viewProjType;
        view->Label.setValue(label);

        this->addView(view);

        this->execute();
        return view;

    } else if(strcmp(viewProjType, "Top Right")  == 0 ||
              strcmp(viewProjType, "Top Left")  == 0 ||
              strcmp(viewProjType, "Bottom Right")  == 0 ||
              strcmp(viewProjType, "Bottom Left")  == 0) {
        // Add an isometric view of the part
    }
    return 0;
}


int FeatureViewOrthographic::removeOrthoView(const char *viewProjType)
{
    // Find a more elegant way of validating the type
    if(strcmp(viewProjType, "Front")  == 0 ||
       strcmp(viewProjType, "Left")   == 0 ||
       strcmp(viewProjType, "Right")  == 0 ||
       strcmp(viewProjType, "Top")    == 0 ||
       strcmp(viewProjType, "Bottom") == 0 ||
       strcmp(viewProjType, "Rear")   == 0 ) {

        if(!hasOrthoView(viewProjType)) {
            throw Base::Exception("The orthographic projection doesn't exist in the group");
        }

        // Iterate through the child views and find the projection type
        const std::vector<App::DocumentObject *> &views = Views.getValues();
        for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {

            Drawing::FeatureView *view = dynamic_cast<Drawing::FeatureView *>(*it);
            if(view->getTypeId() == Drawing::FeatureOrthoView::getClassTypeId()) {
                Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(*it);

                if(strcmp(viewProjType, orthoView->Type.getValueAsString()) == 0) {
                    // Remove from the document
                    this->getDocument()->remObject((*it)->getNameInDocument());
                    return views.size();
                }
            }
        }
    } else if(strcmp(viewProjType, "Top Right")  == 0 ||
              strcmp(viewProjType, "Top Left")  == 0 ||
              strcmp(viewProjType, "Bottom Right")  == 0 ||
              strcmp(viewProjType, "Bottom Left")  == 0) {
        // Remove an isometric view of the part
    }
    return -1;
}

void FeatureViewOrthographic::onDocumentRestored()
{
    this->execute();
}

App::DocumentObjectExecReturn *FeatureViewOrthographic::execute(void)
{
    this->touch();

    if(strcmp(ScaleType.getValueAsString(), "Automatic") == 0) {
        //Recalculate scale

        double autoScale = calculateAutomaticScale();

        if(std::abs(this->Scale.getValue() - autoScale) > FLT_EPSILON) {
            // Set this Scale
            this->Scale.setValue(autoScale);

            //Rebuild the view
            const std::vector<App::DocumentObject *> &views = Views.getValues();
            for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {
                App::DocumentObject *docObj = *it;
                if(docObj->getTypeId().isDerivedFrom(Drawing::FeatureView::getClassTypeId())) {
                    Drawing::FeatureView *view = dynamic_cast<Drawing::FeatureView *>(*it);

                    //Set scale factor of each view
                    view->ScaleType.setValue("Custom");
                    view->Scale.setValue(autoScale);
                    view->Scale.touch();
                    view->Scale.StatusBits.set(2);
                    view->touch();
                }
            }
        }
    }
    return FeatureViewCollection::execute();
}


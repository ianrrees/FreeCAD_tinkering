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
#include "FeatureViewOrthographic.h"

using namespace Drawing;

const char* FeatureViewOrthographic::ProjectionTypeEnums[] = {"Document",
                                                              "First Angle",
                                                              "Third Angle",
                                                              NULL};

PROPERTY_SOURCE(Drawing::FeatureViewOrthographic, Drawing::FeatureViewCollection)

FeatureViewOrthographic::FeatureViewOrthographic(void)
{
    static const char *group = "Drawing view";

    ADD_PROPERTY_TYPE(Anchor, (0), group, App::Prop_None, "The root view to align projections with");

    ProjectionType.setEnums(ProjectionTypeEnums);
    ADD_PROPERTY(ProjectionType, ((long)0));
    ADD_PROPERTY(viewOrientationMatrix, (Base::Matrix4D()));
}

FeatureViewOrthographic::~FeatureViewOrthographic()
{
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
    Drawing::FeatureOrthoView *anchorView = dynamic_cast<Drawing::FeatureOrthoView *>(Anchor.getValue());
    for (std::vector<App::DocumentObject*>::const_iterator it = views.begin(); it != views.end(); ++it) {
         if ((*it)->getTypeId().isDerivedFrom(FeatureViewPart::getClassTypeId())) {
            FeatureViewPart *part = static_cast<FeatureViewPart *>(*it);
            Base::BoundBox3d  bb = part->getBoundingBox();

            bb.ScaleX(1. / part->Scale.getValue());
            bb.ScaleY(1. / part->Scale.getValue());

            // X and Y of dependant views are relative to the anchorView
            if (part != anchorView) {
                bb.MoveX(part->X.getValue());
                bb.MoveY(part->Y.getValue());
            }
            
            bbox.Add(bb);
        }
    }
    // This /should/ leave the centre of the bounding box at (0,0) except when
    // we're in the process of updating the anchor view's position (eg called
    // by moveToCentre())
    if (anchorView) { //TODO: It looks like we might be getting called before an anchor view is set - weird...
        bbox.MoveX(anchorView->X.getValue());
        bbox.MoveY(anchorView->Y.getValue());
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

    Base::BoundBox3d bbox = getBoundingBox();

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

void FeatureViewOrthographic::onChanged(const App::Property* prop)
{
    if (prop == &ProjectionType ||
        prop == &ScaleType ||
        prop == &viewOrientationMatrix ||
        prop == &Scale && !Scale.StatusBits.test(5)){
          if (!isRestoring()) {
              execute();
          }
    }
    Drawing::FeatureViewCollection::onChanged(prop);
}

void FeatureViewOrthographic::moveToCentre(void) 
{
    // Update the anchor view's X and Y to keep the bounding box centred on the origin
    Base::BoundBox3d tempbbox = getBoundingBox();
    FeatureOrthoView *anchorView = dynamic_cast<FeatureOrthoView *>(Anchor.getValue());
    if (anchorView) {
        anchorView->X.setValue((tempbbox.MinX + tempbbox.MaxX) / -2.0);
        anchorView->Y.setValue((tempbbox.MinY + tempbbox.MaxY) / -2.0);
    }
}

App::DocumentObject * FeatureViewOrthographic::getOrthoView(const char *viewProjType) const
{
    const std::vector<App::DocumentObject *> &views = Views.getValues();
    for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {

        FeatureView *view = dynamic_cast<FeatureView *>(*it);
        if(view->getTypeId() == FeatureOrthoView::getClassTypeId()) {
            FeatureOrthoView *orthoView = dynamic_cast<FeatureOrthoView *>(*it);

            if( strcmp(viewProjType, orthoView->Type.getValueAsString()) == 0 )
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

            if( strcmp(viewProjType, orthoView->Type.getValueAsString()) == 0 ) {
                return true;
            }
        }
    }
    return false;
}

bool FeatureViewOrthographic::checkViewProjType(const char *in)
{
    if ( strcmp(in, "Front") == 0 ||
         strcmp(in, "Left") == 0 ||
         strcmp(in, "Right") == 0 ||
         strcmp(in, "Top") == 0 ||
         strcmp(in, "Bottom") == 0 ||
         strcmp(in, "Rear") == 0 ||
         strcmp(in, "FrontTopLeft") == 0 ||
         strcmp(in, "FrontTopRight") == 0 ||
         strcmp(in, "FrontBottomLeft") == 0 ||
         strcmp(in, "FrontBottomRight") == 0) {
        return true;
    }
    return false;
}

App::DocumentObject * FeatureViewOrthographic::addOrthoView(const char *viewProjType)
{
    FeatureOrthoView *view = NULL;

    if ( checkViewProjType(viewProjType) ) {
        if(hasOrthoView(viewProjType)) {
            throw Base::Exception("The Projection is already used in this group");
        }

        std::string FeatName = getDocument()->getUniqueObjectName("OrthoView");
        App::DocumentObject *docObj = getDocument()->addObject("Drawing::FeatureOrthoView",
                                                               FeatName.c_str());

        view = dynamic_cast<Drawing::FeatureOrthoView *>( docObj );
        view->Source.setValue( Source.getValue() );
        view->Scale.setValue( Scale.getValue() );
        view->Type.setValue( viewProjType );
        view->Label.setValue( viewProjType );
        setViewOrientation( view, viewProjType );
    }

    addView(view);         //from FeatureViewCollection
    moveToCentre();

    return view;
}

void FeatureViewOrthographic::setViewOrientation(FeatureOrthoView *v, const char *projType) const
{
    Base::Vector3d dir, xDir;

    // Traditional orthographic
    if(strcmp(projType, "Front") == 0) {
        dir.Set(0, 1, 0);
        xDir.Set(1, 0, 0);

    } else if(strcmp(projType, "Rear") == 0) {
        dir.Set(0, -1, 0);
        xDir.Set(-1, 0, 0);

    } else if(strcmp(projType, "Right") == 0) {
        dir.Set(1, 0, 0);
        xDir.Set(0, -1, 0);

    } else if(strcmp(projType, "Left") == 0) {
        dir.Set(-1, 0, 0);
        xDir.Set(0, 1, 0);

    } else if(strcmp(projType, "Top") == 0) {
        dir.Set(0, 0, 1);
        xDir.Set(1, 0, 0);

    } else if(strcmp(projType, "Bottom") == 0) {
        dir.Set(0, 0, -1);
        xDir.Set(1, 0, 0);

    // Isometric
    } else if(strcmp(projType, "FrontTopLeft") == 0) {
        dir.Set(-1/sqrt(3), 1/sqrt(3), 1/sqrt(3));
        xDir.Set(sqrt(2)/2.0, sqrt(2.0)/2.0, 0);

    } else if(strcmp(projType, "FrontTopRight") == 0) {
        dir.Set(1/sqrt(3), 1/sqrt(3), 1/sqrt(3));
        xDir.Set(sqrt(2)/2.0, -sqrt(2.0)/2.0, 0);

    } else if(strcmp(projType, "FrontBottomRight") == 0) {
        dir.Set(1/sqrt(3), 1/sqrt(3), -1/sqrt(3));
        xDir.Set(sqrt(2)/2.0, -sqrt(2.0)/2.0, 0);

    } else if(strcmp(projType, "FrontBottomLeft") == 0) {
        dir.Set(-1/sqrt(3), 1/sqrt(3), -1/sqrt(3));
        xDir.Set(sqrt(2)/2.0, sqrt(2.0)/2.0, 0);

    } else {
        throw Base::Exception("Unknown view type in FeatureViewOrthographic::setViewOrientation()");
    }

    dir = viewOrientationMatrix.getValue() * dir;
    xDir = viewOrientationMatrix.getValue() * xDir;

    v->Direction.setValue(dir);
    v->XAxisDirection.setValue(xDir);
}

int FeatureViewOrthographic::removeOrthoView(const char *viewProjType)
{
    if ( checkViewProjType(viewProjType) ) {
        if(!hasOrthoView(viewProjType)) {
            throw Base::Exception("The orthographic projection doesn't exist in the group");
        }

        // Iterate through the child views and find the projection type
        const std::vector<App::DocumentObject *> &views = Views.getValues();
        for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {

            Drawing::FeatureView *view = dynamic_cast<Drawing::FeatureView *>(*it);
            if(view->getTypeId() == Drawing::FeatureOrthoView::getClassTypeId()) {
                Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(*it);

                if ( strcmp(viewProjType, orthoView->Type.getValueAsString()) == 0 ) {
                    // Remove from the document
                    getDocument()->remObject((*it)->getNameInDocument());
                    moveToCentre();
                    return views.size();
                }
            }
        }
    }

    return -1;
}

bool FeatureViewOrthographic::distributeOrthoViews()
{
    FeatureOrthoView *anchorView = dynamic_cast<FeatureOrthoView *>(Anchor.getValue());

    if (!anchorView) {  //TODO: Consider not requiring an anchor view, or allowing ones other than "Front"
        return false;
    }

    // Determine layout - should be either "First Angle" or "Third Angle"
    const char* projType;
    if (ProjectionType.isValue("Document")) {
        projType = findParentPage()->OrthoProjectionType.getValueAsString();
    } else {
        projType = ProjectionType.getValueAsString();
    }

    // Setup array of pointers to the views that we're displaying,
    // assuming front is in centre (index 4):
    // [0]  [1]  [2]
    // [3]  [4]  [5]  [6]
    // [7]  [8]  [9]
    FeatureOrthoView * viewPtrs[10] = {};

    // Iterate through views and populate viewPtrs
    FeatureOrthoView* oView;
    std::vector<App::DocumentObject *> views = Views.getValues();
    if ( strcmp(projType, "Third Angle") == 0 ||
         strcmp(projType, "First Angle") == 0    ) {
        //   Third Angle:  FTL  T  FTRight
        //                  L   F   Right   Rear
        //                 FBL  B  FBRight
        //
        //   First Angle:  FBRight  B  FBL
        //                  Right   F   L  Rear
        //                 FTRight  T  FTL
        bool thirdAngle = (strcmp(projType, "Third Angle") == 0);
        for (std::vector<App::DocumentObject*>::const_iterator it = views.begin(); it != views.end(); ++it) {
            if ((*it)->getTypeId().isDerivedFrom(FeatureOrthoView::getClassTypeId())) {
                oView = dynamic_cast<FeatureOrthoView *>(*it);

                const char *viewTypeCStr = oView->Type.getValueAsString();
                if (strcmp(viewTypeCStr, "Front") == 0) {
                    viewPtrs[thirdAngle ? 4 : 4] = oView;
                } else if (strcmp(viewTypeCStr, "Left") == 0) {
                    viewPtrs[thirdAngle ? 3 : 5] = oView;
                } else if (strcmp(viewTypeCStr, "Right") == 0) {
                    viewPtrs[thirdAngle ? 5 : 3] = oView;
                } else if (strcmp(viewTypeCStr, "Top") == 0) {
                    viewPtrs[thirdAngle ? 1 : 8] = oView;
                } else if (strcmp(viewTypeCStr, "Bottom") == 0) {
                    viewPtrs[thirdAngle ? 8 : 1] = oView;
                } else if (strcmp(viewTypeCStr, "Rear") == 0) {
                    viewPtrs[thirdAngle ? 6 : 6] = oView;
                } else if (strcmp(viewTypeCStr, "FrontTopLeft") == 0) {
                    viewPtrs[thirdAngle ? 0 : 9] = oView;
                } else if (strcmp(viewTypeCStr, "FrontTopRight") == 0) {
                    viewPtrs[thirdAngle ? 2 : 7] = oView;
                } else if (strcmp(viewTypeCStr, "FrontBottomLeft") == 0) {
                    viewPtrs[thirdAngle ? 7 : 2] = oView;
                } else if (strcmp(viewTypeCStr, "FrontBottomRight") == 0) {
                    viewPtrs[thirdAngle ? 9 : 0] = oView;
                } else {
                    throw Base::Exception("Unknown view type in FeatureViewOrthographic::distributeOrthoViews()");
                }
            }
        }
    } else return false;    // ...assuming this is an error condition

    // TODO: Work on not requiring the front view...
    if (!viewPtrs[4])
        return false;

    // Calculate bounding boxes for each displayed view
    Base::BoundBox3d bboxes[10];

    for (int i = 0; i < 10; ++i)
        if (viewPtrs[i]) {
            bboxes[i] = viewPtrs[i]->getBoundingBox();
        } else {
            // BoundBox3d defaults to length=(FLOAT_MAX + -FLOAT_MAX)
            bboxes[i].ScaleX(0);
            bboxes[i].ScaleY(0);
            bboxes[i].ScaleZ(0);
        }

    // Now that things are setup, do the spacing
    double spacing = 15;    //in mm  TODO: maybe use a property?

    if (viewPtrs[0]) {
        double displace = std::max(bboxes[0].LengthX() + bboxes[4].LengthX(),
                                   bboxes[0].LengthY() + bboxes[4].LengthY());
        viewPtrs[0]->X.setValue(displace / -2.0 - spacing);
        viewPtrs[0]->Y.setValue(displace / 2.0 + spacing);
    }
    if (viewPtrs[1]) {
        viewPtrs[1]->Y.setValue((bboxes[1].LengthY() + bboxes[4].LengthY()) / 2.0 + spacing);
    }
    if (viewPtrs[2]) {
        double displace = std::max(bboxes[2].LengthX() + bboxes[4].LengthX(),
                                   bboxes[2].LengthY() + bboxes[4].LengthY());
        viewPtrs[2]->X.setValue(displace / 2.0 + spacing);
        viewPtrs[2]->Y.setValue(displace / 2.0 + spacing);
    }
    if (viewPtrs[3]) {
        viewPtrs[3]->X.setValue((bboxes[3].LengthX() + bboxes[4].LengthX()) / -2.0 - spacing);
    }
    if (viewPtrs[4]) {  // TODO: Move this check above, and figure out a sane bounding box based on other existing views
    }
    if (viewPtrs[5]) {
        viewPtrs[5]->X.setValue((bboxes[5].LengthX() + bboxes[4].LengthX()) / 2.0 + spacing);
    }
    if (viewPtrs[6]) {
        if (viewPtrs[5])
            viewPtrs[6]->X.setValue((bboxes[6].LengthX() + bboxes[4].LengthX()) / 2.0 + bboxes[5].LengthX() + 2 * spacing);
        else
            viewPtrs[6]->X.setValue((bboxes[6].LengthX() + bboxes[4].LengthX()) / 2.0 + spacing);
    }
    if (viewPtrs[7]) {
        double displace = std::max(bboxes[7].LengthX() + bboxes[4].LengthX(),
                                   bboxes[7].LengthY() + bboxes[4].LengthY());
        viewPtrs[7]->X.setValue(displace / -2.0 - spacing);
        viewPtrs[7]->Y.setValue(displace / -2.0 - spacing);
    }
    if (viewPtrs[8]) {
        viewPtrs[8]->Y.setValue((bboxes[8].LengthY() + bboxes[4].LengthY()) / -2.0 - spacing);
    }
    if (viewPtrs[9]) {
        double displace = std::max(bboxes[9].LengthX() + bboxes[4].LengthX(),
                                   bboxes[9].LengthY() + bboxes[4].LengthY());
        viewPtrs[9]->X.setValue(displace / 2.0 + spacing);
        viewPtrs[9]->Y.setValue(displace / -2.0 - spacing);
    }

    return true;
}

//TODO: Turn this into a command so it can be issued from python
void FeatureViewOrthographic::setFrontViewOrientation(const Base::Matrix4D &newMat)
{
    viewOrientationMatrix.setValue(newMat);

    FeatureOrthoView *view;
    std::vector<App::DocumentObject *> views = Views.getValues();
    for (std::vector<App::DocumentObject*>::const_iterator it = views.begin(); it != views.end(); ++it) {
        if ((*it)->getTypeId().isDerivedFrom(FeatureOrthoView::getClassTypeId())) {
            view = dynamic_cast<FeatureOrthoView *>(*it);
            setViewOrientation(view, view->Type.getValueAsString());
            view->touch();
        }
    }
}

App::DocumentObjectExecReturn *FeatureViewOrthographic::execute(void)
{
    if(strcmp(ScaleType.getValueAsString(), "Automatic") == 0) {
        //Recalculate scale

        double autoScale = calculateAutomaticScale();

        if(std::abs(Scale.getValue() - autoScale) > FLT_EPSILON) {
            // Set this Scale
            Scale.setValue(autoScale);

            //Rebuild the view
            const std::vector<App::DocumentObject *> &views = Views.getValues();
            for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {
                App::DocumentObject *docObj = *it;
                if(docObj->getTypeId().isDerivedFrom(FeatureView::getClassTypeId())) {
                    FeatureView *view = dynamic_cast<FeatureView *>(*it);

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

    // recalculate positions for children
    if (Views.getSize()) {
        distributeOrthoViews();
    }
    //touch();

    return FeatureViewCollection::execute();
}

void FeatureViewOrthographic::onDocumentRestored()
{
    execute();
}


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

#ifndef _DRAWING_FEATUREVIEWORTHOGRAPHIC_h_
#define _DRAWING_FEATUREVIEWORTHOGRAPHIC_h_

#include <App/DocumentObject.h>
#include <App/PropertyStandard.h>
#include <App/FeaturePython.h>

#include <Base/BoundBox.h>
#include "FeatureViewCollection.h"

namespace Drawing
{

/**
 * Class super-container for managing a collection of FeatureOrthoView
 * Page Features
 */
class DrawingExport FeatureViewOrthographic : public Drawing::FeatureViewCollection
{
    PROPERTY_HEADER(Drawing::FeatureViewOrthographic);

public:
    /// Constructor
    FeatureViewOrthographic();
    ~FeatureViewOrthographic();

    App::PropertyEnumeration ProjectionType;
    App::PropertyLink Anchor; /// Anchor Element to align views to

public:
    Base::BoundBox3d getBoundingBox() const;
    double calculateAutomaticScale() const;

    // Check if container has a view of a specific type
    bool hasOrthoView(const char *viewProjType) const;
    App::DocumentObject * getOrthoView(const char *viewProjType) const;

    App::DocumentObject * addOrthoView(const char *viewProjType);

    //! Removes an ortho view
    /*!
     * \return number of ortho views remaining
     */
    int  removeOrthoView(const char *viewProjType);

    bool distributeOrthoViews(void);

    short mustExecute() const;
    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    virtual void onDocumentRestored();
    virtual App::DocumentObjectExecReturn *execute(void);
    //@}

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "DrawingGui::ViewProviderViewOrthographic";
    }

    // TODO: Add isometric
    // Keep this in sync with orthoViewNameFromStr and OrthoViewNameEnumStrs
    enum OrthoViewNameEnum {FRONT=0, LEFT, RIGHT, TOP, BOTTOM, REAR, ERROR};
protected:
    void onChanged(const App::Property* prop);
    OrthoViewNameEnum orthoViewNameFromStr(const char *inStr);

    //! Moves anchor view to keep our bounding box centre on the origin
    void moveToCentre();
private:
    static const char* ProjectionTypeEnums[];
    static const char* OrthoViewNameEnumStrs[];

};

} //namespace Drawing

#endif // _DRAWING_FEATUREVIEWORTHOGRAPHIC_h_

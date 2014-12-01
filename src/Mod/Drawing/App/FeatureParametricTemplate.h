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

#ifndef _DRAWING_FeatureParametricTemplate_h_
#define _DRAWING_FeatureParametricTemplate_h_

#include <App/PropertyFile.h>
#include <App/FeaturePython.h>

#include "FeatureTemplate.h"

namespace DrawingGeometry
{
    class BaseGeom;
}

namespace Drawing
{

/** Base class of all View Features in the drawing module
 */
class DrawingExport FeatureParametricTemplate: public Drawing::FeatureTemplate
{
    PROPERTY_HEADER(Drawing::FeatureParametricTemplate);

public:
    FeatureParametricTemplate(); /// Constructor
    ~FeatureParametricTemplate();

    App::PropertyFile Template;

    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    virtual App::DocumentObjectExecReturn *execute(void);
    //@}


    short mustExecute() const;

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "DrawingGui::ViewProviderTemplate";
    }

    // from base class
    virtual PyObject *getPyObject(void);
    virtual unsigned int getMemSize(void) const;

public:
    std::vector<DrawingGeometry::BaseGeom *> getGeometry() { return geom; }
    int clearGeometry();

    // Template Drawing Methods
    int drawLine(double x1, double y1, double x2, double y2);

    double getHeight() const;
    double getWidth() const;

protected:
    void onChanged(const App::Property* prop);

protected:
    std::vector<DrawingGeometry::BaseGeom *> geom;

private:
    static const char* OrientationEnums[];
};

typedef App::FeaturePythonT<FeatureParametricTemplate> FeatureParametricTemplatePython;

} //namespace Drawing



#endif //_DRAWING_FeatureParametricTemplate_h_

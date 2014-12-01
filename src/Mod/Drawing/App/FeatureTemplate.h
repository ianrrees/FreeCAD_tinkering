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

#ifndef _DRAWING_FeatureTemplate_h_
#define _DRAWING_FeatureTemplate_h_

#include <App/DocumentObject.h>

#include <App/PropertyLinks.h>
#include <App/PropertyStandard.h>
#include <App/PropertyUnits.h>
#include <App/FeaturePython.h>

namespace DrawingGeometry
{
    class BaseGeom;
}

namespace Drawing
{

/** Base class of all View Features in the drawing module
 */
class DrawingExport FeatureTemplate: public App::DocumentObject
{
    PROPERTY_HEADER(Drawing::FeatureTemplate);

public:
    FeatureTemplate(); /// Constructor
    ~FeatureTemplate();

    // Page Physical Properties
    App::PropertyLength Width;
    App::PropertyLength Height;
    App::PropertyEnumeration Orientation;
    App::PropertyString PaperSize;

public:

    // virtual functions must be redefined by base class
    virtual double getWidth() const;
    virtual double getHeight() const;

    virtual void getBlockDimensions(double &x, double &y, double &width, double &height) const;

    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    virtual App::DocumentObjectExecReturn *execute(void);
    //@}


    virtual short mustExecute() const;

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "DrawingGui::ViewProviderTemplate";
    }

    // from base class
    virtual PyObject *getPyObject(void);
    virtual unsigned int getMemSize(void) const;

protected:
    void onChanged(const App::Property* prop);

private:
    static const char* OrientationEnums[];

};

typedef App::FeaturePythonT<FeatureTemplate> FeatureTemplatePython;

} //namespace Drawing



#endif //_DRAWING_FeatureTemplate_h_

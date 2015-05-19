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

#ifndef _Drawing_FeatureViewDimension_h_
#define _Drawing_FeatureViewDimension_h_

# include <App/DocumentObject.h>
# include <App/FeaturePython.h>
# include <App/PropertyLinks.h>

#include "FeatureView.h"
#include "FeatureViewPart.h"

namespace Measure {
class Measurement;
}
namespace Drawing
{

/** Base class of all View Features in the drawing module
 */
class DrawingExport FeatureViewDimension : public Drawing::FeatureView
{
    PROPERTY_HEADER(Drawing::FeatureViewDimension);

public:
    /// Constructor
    FeatureViewDimension();
    virtual ~FeatureViewDimension();

  //     App::PropertyLinkSubList View;
    App::PropertyEnumeration ProjectionType;
    App::PropertyVector ProjDirection;
    App::PropertyLinkSubList References;
    App::PropertyEnumeration Type;
    App::PropertyVector XAxisDirection;

    /// Properties for Visualisation
    App::PropertyInteger Precision;
    App::PropertyString  Font;
    App::PropertyFloat   Fontsize;
    App::PropertyBool    CentreLines;
    App::PropertyString  FormatSpec;

    short mustExecute() const;

    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    virtual App::DocumentObjectExecReturn *execute(void);
    //@}

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "DrawingGui::ViewProviderDimension";
    }

    virtual std::string getFormatedValue() const;
    virtual double getDimValue() const;
    FeatureViewPart* getViewPart() const;

protected:
    void onChanged(const App::Property* prop);
    
protected:
    Measure::Measurement *measurement;

private:
    static const char* TypeEnums[];
    static const char* ProjTypeEnums[];
};

} //namespace Drawing
#endif

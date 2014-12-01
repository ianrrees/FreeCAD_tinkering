/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2007     *
 *   Copyright (c) Luke Parry             (l.parry@warwick.ac.uk) 2013     *
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

#ifndef _FeatureViewSection_h_
#define _FeatureViewSection_h_

#include <App/DocumentObject.h>
#include <App/PropertyLinks.h>
#include <App/FeaturePython.h>

#include "FeatureViewPart.h"

class gp_Pln;

namespace Drawing
{


/** Base class of all View Features in the drawing module
 */
class DrawingExport FeatureViewSection : public FeatureViewPart
{
    PROPERTY_HEADER(Part::FeatureViewSection);

public:
    /// Constructor
    FeatureViewSection(void);
    virtual ~FeatureViewSection();

    App::PropertyVector SectionNormal;
    App::PropertyVector SectionOrigin;
    App::PropertyBool ShowCutSurface;

    short mustExecute() const;

    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    virtual App::DocumentObjectExecReturn *execute(void);
    //@}

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "DrawingGui::ViewProviderDrawingViewSection";
    }

public:
    void getSectionSurface(std::vector<DrawingGeometry::Face *> &sectionFace) const;

protected:
    TopoDS_Shape result;
    gp_Pln getSectionPlane() const;
};

typedef App::FeaturePythonT<FeatureViewSection> FeatureViewSectionPython;


} //namespace Drawing

#endif
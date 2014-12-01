/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2007     *
 *   Copyright (c) 2013 Luke Parry        <l.parry@warwick.ac.uk>          *
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

#ifndef _DRAWING_FEATUREVIEWCOLLECTION_h_
#define _DRAWING_FEATUREVIEWCOLLECTION_h_

#include <App/DocumentObject.h>
#include <App/PropertyLinks.h>

#include "FeatureView.h"

namespace Drawing
{

/** Base class for collection of view objects
 */
class DrawingExport FeatureViewCollection : public FeatureView
{
    PROPERTY_HEADER(Drawing::FeatureViewCollection);

public:
    App::PropertyLink     Source;
    App::PropertyLinkList Views;
public:
    /// Constructor
    FeatureViewCollection();
    virtual ~FeatureViewCollection();
    short mustExecute() const;

    int addView(FeatureView *view);

    int countChildren();
    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    virtual void onDocumentRestored();
    virtual App::DocumentObjectExecReturn *execute(void);
    //@}

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "DrawingGui::ViewProviderViewCollection";
    }

protected:
    void onChanged(const App::Property* prop);
};

} //namespace Drawing

#endif // _DRAWING_FEATUREVIEWCOLLECTION_h_

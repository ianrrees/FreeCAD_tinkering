/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                                                                         *
 *   This file is Drawing of the FreeCAD CAx development system.           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A DrawingICULAR PURPOSE.  See the      *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef DRAWINGGUI_VIEWPROVIDERVIEWORTHOGRAPHIC_H
#define DRAWINGGUI_VIEWPROVIDERVIEWORTHOGRAPHIC_H

#include <Gui/ViewProviderFeature.h>
#include <Gui/ViewProviderDocumentObject.h>

#include <QPointer>

namespace Drawing{
    class FeatureViewOrthographic;
}

namespace DrawingGui {


class DrawingGuiExport ViewProviderViewOrthographic : public Gui::ViewProviderDocumentObject
{
    PROPERTY_HEADER(DrawingGui::ViewProviderViewOrthographic);

public:

     ViewProviderViewOrthographic();  /// constructor
     ~ViewProviderViewOrthographic(); /// destructor

    virtual void attach(App::DocumentObject *);
    virtual void setDisplayMode(const char* ModeName);
    virtual bool useNewSelectionModel(void) const {return false;}
    /// returns a list of all possible modes
    virtual std::vector<std::string> getDisplayModes(void) const;

    /// Claim all the views for the page
    std::vector<App::DocumentObject*> claimChildren(void) const;

    /// Is called by the tree if the user double click on the object
    virtual bool doubleClicked(void);
    void setupContextMenu(QMenu*, QObject*, const char*);
    virtual void updateData(const App::Property*);

    Drawing::FeatureViewOrthographic* getObject() const;
    void unsetEdit(int ModNum);

protected:
    bool setEdit(int ModNum);

};

} // namespace DrawingGui

#endif // DRAWINGGUI_VIEWPROVIDERVIEWORTHOGRAPHIC_H

/***************************************************************************
 *   Copyright (c) 2014 Luke Parry    <l.parry@warwick.ac.uk>              *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      *
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
#endif

/// Here the FreeCAD includes sorted by Base,App,Gui......
#include <Base/Console.h>
#include <Base/Parameter.h>
#include <Base/Exception.h>
#include <Base/Sequencer.h>

#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>

#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/Command.h>
#include <Gui/Control.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/Selection.h>
#include <Gui/SoFCSelection.h>


#include <Mod/Drawing/App/FeatureOrthoView.h>

#include "ViewProviderOrthoView.h"

using namespace DrawingGui;

PROPERTY_SOURCE(DrawingGui::ViewProviderOrthoView, DrawingGui::ViewProviderViewPart)

//**************************************************************************
// Construction/Destruction

ViewProviderOrthoView::ViewProviderOrthoView()
{

}

ViewProviderOrthoView::~ViewProviderOrthoView()
{
}

void ViewProviderOrthoView::attach(App::DocumentObject *pcFeat)
{
    // call parent attach method
    ViewProviderDocumentObject::attach(pcFeat);
}

void ViewProviderOrthoView::setDisplayMode(const char* ModeName)
{
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}

std::vector<std::string> ViewProviderOrthoView::getDisplayModes(void) const
{
    // get the modes of the father
    std::vector<std::string> StrList = ViewProviderDocumentObject::getDisplayModes();
    StrList.push_back("Drawing");
    return StrList;
}

void ViewProviderOrthoView::updateData(const App::Property* prop)
{
    Gui::ViewProviderDocumentObject::updateData(prop);


    Drawing::FeatureOrthoView* ortho = getObject();

    if(ortho) {


        // Set the icon pixmap depending on the orientation
        std::string projType = ortho->Type.getValueAsString();

        if(strcmp(projType.c_str(), "Front") == 0) {
            sPixmap = "OrthoFront";
        } else if(strcmp(projType.c_str(), "Rear") == 0) {
            sPixmap = "OrthoRear";
        } else if(strcmp(projType.c_str(), "Right") == 0) {
            sPixmap = "OrthoRight";
        } else if(strcmp(projType.c_str(), "Left") == 0) {
           sPixmap = "OrthoLeft";
        } else if(strcmp(projType.c_str(), "Top") == 0) {
            sPixmap = "OrthoTop";
        } else if(strcmp(projType.c_str(), "Bottom") == 0) {
           sPixmap = "OrthoBottom";
        }
    }
 }


void ViewProviderOrthoView::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    QAction* act;
    act = menu->addAction(QObject::tr("Show drawing"), receiver, member);
}

bool ViewProviderOrthoView::setEdit(int ModNum)
{
    doubleClicked();
    return true;
}

void ViewProviderOrthoView::unsetEdit(int ModNum)
{
    Gui::Control().closeDialog();
}

bool ViewProviderOrthoView::doubleClicked(void)
{
    return true;
}

bool ViewProviderOrthoView::onDelete(const std::vector<std::string> &subList)
{

    Gui::Command::doCommand(Gui::Command::Doc,"App.ActiveDocument.removeObject(\"%s\")"
                                             ,getObject()->getNameInDocument());
    Gui::Command::commitCommand();
    Gui::Command::updateActive();
    return false;

}

Drawing::FeatureOrthoView* ViewProviderOrthoView::getObject() const
{
    return dynamic_cast<Drawing::FeatureOrthoView*>(pcObject);
}

/***************************************************************************
 *   Copyright (c) 2013 Luke Parry    <l.parry@warwick.ac.uk>              *
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

#include "PreCompiled.h"
#ifndef _PreComp_
# ifdef FC_OS_WIN32
#  include <windows.h>
# endif
# include <QAction>
# include <QMenu>
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
#include <Gui/Control.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/MainWindow.h>
#include <Gui/Selection.h>
#include <Gui/SoFCSelection.h>
#include <Gui/ViewProviderDocumentObject.h>

#include <Mod/Drawing/App/FeatureViewOrthographic.h>

#include "TaskOrthographicViews.h"
#include "ViewProviderViewOrthographic.h"

using namespace DrawingGui;

PROPERTY_SOURCE(DrawingGui::ViewProviderViewOrthographic, Gui::ViewProviderDocumentObject)

//**************************************************************************
// Construction/Destruction

ViewProviderViewOrthographic::ViewProviderViewOrthographic()
{
    sPixmap = "OrthoCollection";
}

ViewProviderViewOrthographic::~ViewProviderViewOrthographic()
{
}

void ViewProviderViewOrthographic::attach(App::DocumentObject *pcFeat)
{
    // call parent attach method
    ViewProviderDocumentObject::attach(pcFeat);
}

void ViewProviderViewOrthographic::setDisplayMode(const char* ModeName)
{
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}

std::vector<std::string> ViewProviderViewOrthographic::getDisplayModes(void) const
{
    // get the modes of the father
    std::vector<std::string> StrList = ViewProviderDocumentObject::getDisplayModes();
    StrList.push_back("Drawing");
    return StrList;
}

void ViewProviderViewOrthographic::updateData(const App::Property* prop)
{
    Gui::ViewProviderDocumentObject::updateData(prop);

    if(prop == &(getObject()->Scale) ||
       prop == &(getObject()->ScaleType) ||
       prop == &(getObject()->Views) ||
       prop == &(getObject()->ProjectionType)) {

        Gui::TaskView::TaskDialog *dlg = Gui::Control().activeDialog();
        TaskDlgOrthographicViews *orthoDlg = qobject_cast<TaskDlgOrthographicViews *>(dlg);

        if (orthoDlg && orthoDlg->getOrthographicView() != this)
            orthoDlg = 0;

        if(orthoDlg) {
            orthoDlg->update();
        }
    } 

 }


void ViewProviderViewOrthographic::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    QAction* act;
    act = menu->addAction(QObject::tr("Show drawing"), receiver, member);
}

bool ViewProviderViewOrthographic::setEdit(int ModNum)
{

    // When double-clicking on the item for this sketch the
    // object unsets and sets its edit mode without closing
    // the task panel
    Gui::TaskView::TaskDialog *dlg = Gui::Control().activeDialog();
    TaskDlgOrthographicViews *orthoDlg = qobject_cast<TaskDlgOrthographicViews *>(dlg);
    if (orthoDlg && orthoDlg->getOrthographicView() != this)
        orthoDlg = 0; // another sketch left open its task panel

    // clear the selection (convenience)
    Gui::Selection().clearSelection();

    // start the edit dialog
    if (orthoDlg)
        Gui::Control().showDialog(orthoDlg);
    else
        Gui::Control().showDialog(new TaskDlgOrthographicViews(this));

    return true;
}

void ViewProviderViewOrthographic::unsetEdit(int ModNum)
{
    Gui::Control().closeDialog();
}

bool ViewProviderViewOrthographic::doubleClicked(void)
{
    setEdit(0);
    return true;
}

std::vector<App::DocumentObject*> ViewProviderViewOrthographic::claimChildren(void) const
{
    // Collect any child fields and put this in the CamFeature tree
    std::vector<App::DocumentObject*> temp;
    const std::vector<App::DocumentObject *> &views = getObject()->Views.getValues();
    try {
      for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {
          temp.push_back(*it);
      }
      return temp;
    } catch (...) {
        std::vector<App::DocumentObject*> tmp;
        return tmp;
    }
}


Drawing::FeatureViewOrthographic* ViewProviderViewOrthographic::getObject() const
{
    return dynamic_cast<Drawing::FeatureViewOrthographic*>(pcObject);
}

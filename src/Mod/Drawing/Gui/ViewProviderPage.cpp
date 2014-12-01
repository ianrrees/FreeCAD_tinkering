/***************************************************************************
 *   Copyright (c) 2004 Jürgen Riegel <juergen.riegel@web.de>              *
 *   Copyright (c) 2012 Luke Parry    <l.parry@warwick.ac.uk>              *
 *                                                                         *
 *   This file is Drawing of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A DrawingICULAR PURPOSE.  See the         *
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
# include <QAction>
# include <QMenu>
# include <QTimer>
#include <QPointer>
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
#include <Gui/Selection.h>
#include <Gui/MainWindow.h>
#include <Gui/BitmapFactory.h>
#include <Gui/ViewProviderDocumentObjectGroup.h>

#include <Mod/Drawing/App/FeaturePage.h>
#include <Mod/Drawing/App/FeatureView.h>

#include <Mod/Drawing/App/FeatureOrthoView.h>
#include <Mod/Drawing/App/FeatureViewDimension.h>

#include "DrawingView.h"
#include "ViewProviderPage.h"

using namespace DrawingGui;

PROPERTY_SOURCE(DrawingGui::ViewProviderDrawingPage, Gui::ViewProviderDocumentObjectGroup)


//**************************************************************************
// Construction/Destruction

ViewProviderDrawingPage::ViewProviderDrawingPage()
  : view(0)
{
    sPixmap = "Page";

//    ADD_PROPERTY(HintScale,(10.0));
//    ADD_PROPERTY(HintOffsetX,(10.0));
//    ADD_PROPERTY(HintOffsetY,(10.0));

    // do not show this in the property editor
    Visibility.StatusBits.set(3, true);
    DisplayMode.StatusBits.set(3, true);
}

ViewProviderDrawingPage::~ViewProviderDrawingPage()
{
}

void ViewProviderDrawingPage::attach(App::DocumentObject *pcFeat)
{
    // call parent attach method
    ViewProviderDocumentObjectGroup::attach(pcFeat);
}

void ViewProviderDrawingPage::setDisplayMode(const char* ModeName)
{
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}

std::vector<std::string> ViewProviderDrawingPage::getDisplayModes(void) const
{
    // get the modes of the father
    std::vector<std::string> StrList = ViewProviderDocumentObject::getDisplayModes();
    StrList.push_back("Drawing");
    return StrList;
}

void ViewProviderDrawingPage::onChanged(const App::Property *prop)
{
  if (prop == &(getPageObject()->Views)) {
        if(this->view) {
            view->updateDrawing();
        }
    } else if (prop == &(getPageObject()->Template)) {
       if(this->view) {
            view->updateTemplate();
        }
    }

    Gui::ViewProviderDocumentObjectGroup::onChanged(prop);
}

void ViewProviderDrawingPage::show(void)
{
    // showing the drawing page should not affect its children but opens the MDI view
    // therefore do not call the method of its direct base class
    ViewProviderDocumentObject::show();
    if (!this->view) {
        showDrawingView();
        this->view->load(QString::fromUtf8(getPageObject()->PageResult.getValue()));
        view->viewAll();
    }
}

void ViewProviderDrawingPage::hide(void)
{
    // hiding the drawing page should not affect its children but closes the MDI view
    // therefore do not call the method of its direct base class
    ViewProviderDocumentObject::hide();
    if (view) {
        view->parentWidget()->deleteLater();
    }
}

void ViewProviderDrawingPage::updateData(const App::Property* prop)
{
    if (prop == &(getPageObject()->Views)) {
        if(this->view) {
            view->updateDrawing();
        }
    } else if (prop == &(getPageObject()->Template)) {
       if(this->view) {
            view->updateTemplate();
        }
    }

    Gui::ViewProviderDocumentObjectGroup::updateData(prop);
}

bool ViewProviderDrawingPage::onDelete(const std::vector<std::string> & items)
{
    if (view) {
        view->parentWidget()->deleteLater();
    }

    return ViewProviderDocumentObjectGroup::onDelete(items);
}

void ViewProviderDrawingPage::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    menu->addAction(QObject::tr("Show drawing"), receiver, member);
}

bool ViewProviderDrawingPage::setEdit(int ModNum)
{
    doubleClicked();
    return true;
}

void ViewProviderDrawingPage::unsetEdit(int ModNum)
{
    try {
        // Update the Document
        getPageObject()->getDocument()->recompute();
    }
    catch (...) {
    }

    // clear the selection and set the new/edited sketch(convenience)
    Gui::Selection().clearSelection();
    std::string ObjName = getPageObject()->getNameInDocument();
    std::string DocName = getPageObject()->getDocument()->getName();
    Gui::Selection().addSelection(DocName.c_str(),ObjName.c_str());

    // when pressing ESC make sure to close the dialog
    Gui::Control().closeDialog();
}
bool ViewProviderDrawingPage::doubleClicked(void)
{
//    show();
    if (!this->view) {
        showDrawingView();
        //view->attachPageObject(getPageObject());
        view->updateDrawing();
        view->updateTemplate(true);
        view->viewAll();
    } else {
        view->updateDrawing();
        view->updateTemplate(true);
    }
    Gui::getMainWindow()->setActiveWindow(this->view);
    Gui::Application::Instance->activeDocument()->setEdit(this);
    return true;
}

std::vector<App::DocumentObject*> ViewProviderDrawingPage::claimChildren(void) const
{

    std::vector<App::DocumentObject*> temp;

    // Attach the template if it exists
    App::DocumentObject *templateFeat = 0;
    templateFeat = getPageObject()->Template.getValue();

    if(templateFeat) {
        temp.push_back(templateFeat);
    }

    // Collect any child views
    const std::vector<App::DocumentObject *> &views = getPageObject()->Views.getValues();

    try {
      for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {
          App::DocumentObject *docObj = *it;
          // Don't collect if dimension or ortho view as these should be grouped elsewhere
          if(docObj->isDerivedFrom(Drawing::FeatureOrthoView::getClassTypeId())    ||
             docObj->isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId())  )
              continue;
          else
              temp.push_back(*it);
      }
      return temp;
    } catch (...) {
        std::vector<App::DocumentObject*> tmp;
        return tmp;
    }
}
DrawingView* ViewProviderDrawingPage::showDrawingView()
{
    if (!view){
        Gui::Document* doc = Gui::Application::Instance->getDocument
            (this->pcObject->getDocument());
        view = new DrawingView(this, doc, Gui::getMainWindow());
        view->setWindowIcon(Gui::BitmapFactory().pixmap("actions/drawing-landscape"));

//        const char* objname = pcObject->Label.getValue();
//        view->setObjectName(QString::fromUtf8(objname));
//        view->onRelabel(doc);
//        view->setDocumentObject(pcObject->getNameInDocument());
        view->setWindowTitle(QObject::tr("Drawing viewer") + QString::fromAscii("[*]"));
        Gui::getMainWindow()->addWindow(view);
        view->viewAll();
    }

    // Update the drawing
    return view;
}

void ViewProviderDrawingPage::onSelectionChanged(const Gui::SelectionChanges& msg)
{

    if(view) {
        if(msg.Type == Gui::SelectionChanges::SetSelection) {
            view->clearSelection();
            std::vector<Gui::SelectionSingleton::SelObj> objs = Gui::Selection().getSelection(msg.pDocName);

            for (std::vector<Gui::SelectionSingleton::SelObj>::iterator it = objs.begin(); it != objs.end(); ++it) {
                Gui::SelectionSingleton::SelObj selObj = *it;

                if(selObj.pObject == getPageObject())
                    continue;

                std::string str = msg.pSubName;
                // If it's a subfeature, dont select feature
                if(strcmp(str.substr(0,4).c_str(), "Edge") == 0||
                  strcmp(str.substr(0,6).c_str(), "Vertex") == 0){
                    // TODO implement me
                } else {
                    view->selectFeature(selObj.pObject, true);
                }

            }
        } else {
            bool selectState = (msg.Type == Gui::SelectionChanges::AddSelection) ? true : false;
            Gui::Document* doc = Gui::Application::Instance->getDocument(this->pcObject->getDocument());
            App::DocumentObject *obj = doc->getDocument()->getObject(msg.pObjectName);
            if(obj) {

                std::string str = msg.pSubName;
                // If it's a subfeature, dont select feature
                if(strcmp(str.substr(0,4).c_str(), "Edge") == 0||
                  strcmp(str.substr(0,6).c_str(), "Vertex") == 0){
                    // TODO implement me
                } else {
                    view->selectFeature(obj, selectState);
                }
            }
        }
    }

}

bool ViewProviderDrawingPage::onDelete(const std::vector<std::string> &subList)
{

    Gui::getMainWindow()->removeWindow(view);
    Gui::getMainWindow()->activatePreviousWindow();


    Gui::Selection().clearSelection();
    view->deleteLater(); // Delete the drawing view;
}

Drawing::FeaturePage* ViewProviderDrawingPage::getPageObject() const
{
    return dynamic_cast<Drawing::FeaturePage*>(pcObject);
}

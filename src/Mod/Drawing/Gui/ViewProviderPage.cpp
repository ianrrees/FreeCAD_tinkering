/***************************************************************************
 *   Copyright (c) 2004 Jürgen Riegel <juergen.riegel@web.de>              *
 *   Copyright (c) 2012 Luke Parry    <l.parry@warwick.ac.uk>              *
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
#include <Gui/ViewProviderDocumentObject.h>
#include <Gui/ViewProviderDocumentObjectGroup.h>


#include "DrawingView.h"
#include "ViewProviderPage.h"
#include <Mod/Drawing/App/FeaturePage.h>
#include <Mod/Drawing/App/FeatureView.h>
#include <Mod/Drawing/App/FeatureProjGroupItem.h>
#include <Mod/Drawing/App/FeatureViewDimension.h>

using namespace DrawingGui;

PROPERTY_SOURCE(DrawingGui::ViewProviderDrawingPage, Gui::ViewProviderDocumentObject)


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
    ViewProviderDocumentObject::attach(pcFeat);
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

void ViewProviderDrawingPage::show(void)
{
    showDrawingView();
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
        if(view) {
            view->updateDrawing();
        }
    } else if (prop == &(getPageObject()->Template)) {
       if(view) {
            view->updateTemplate();
        }
    }

    Gui::ViewProviderDocumentObject::updateData(prop);
}

bool ViewProviderDrawingPage::onDelete(const std::vector<std::string> &items)
{
    if (view) {
        // TODO: if DrawingPage has children, they should be deleted too, since they are useless without the page.
        //       logic is in the "this object has links are you sure" dialog 
        Gui::getMainWindow()->removeWindow(view);
        Gui::getMainWindow()->activatePreviousWindow();
        view->deleteLater(); // Delete the drawing view;
    } else {
        // DrawingView is not displayed yet so don't try to delete it!
        Base::Console().Log("INFO - ViewProviderDrawingPage::onDelete - Page object deleted when viewer not displayed\n");
    }
    Gui::Selection().clearSelection();
    return ViewProviderDocumentObject::onDelete(items);
}

void ViewProviderDrawingPage::setupContextMenu(QMenu* menu, QObject* receiver, const char* member)
{
    menu->addAction(QObject::tr("Show drawing"), receiver, member);
    act->setData(QVariant(1));
}

bool ViewProviderDrawingPage::setEdit(int ModNum)
{
    if (ModNum == 1) {
        showDrawingView();   // show the drawing
        Gui::getMainWindow()->setActiveWindow(view);
        return false;
    } else {
        Gui::ViewProviderDocumentObject::setEdit(ModNum);
    }
    return true;
}

bool ViewProviderDrawingPage::doubleClicked(void)
{
    showDrawingView();
    Gui::getMainWindow()->setActiveWindow(view);
    return true;
}

bool ViewProviderDrawingPage::showDrawingView()
{
    if (!view){
        Gui::Document* doc = Gui::Application::Instance->getDocument
            (pcObject->getDocument());
        view = new DrawingView(this, doc, Gui::getMainWindow());
        view->setWindowTitle(QObject::tr("Drawing viewer") + QString::fromAscii("[*]"));
        view->setWindowIcon(Gui::BitmapFactory().pixmap("actions/drawing-landscape"));
        view->updateDrawing(true);
     //   view->updateTemplate(true);   //TODO: I don't think this is necessary?  Ends up triggering a reload of SVG template, but the DrawingView constructor does too.
        Gui::getMainWindow()->addWindow(view);
        view->viewAll();
    } else {
        view->updateDrawing(true);
        view->updateTemplate(true);
    }
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
    // for Page, valid children are any View except: FeatureProjGroupItem
    //                                               FeatureViewDimension
    //                                               any FeatuerView in a FeatureViewClip

    const std::vector<App::DocumentObject *> &views = getPageObject()->Views.getValues();

    try {
      for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {
          Drawing::FeatureView* featView = dynamic_cast<Drawing::FeatureView*> (*it);
          App::DocumentObject *docObj = *it;
          // Don't collect if dimension, projection group item or member of ClipGroup as these should be grouped elsewhere
          if(docObj->isDerivedFrom(Drawing::FeatureProjGroupItem::getClassTypeId())    ||
             docObj->isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId())    ||
             (featView && featView->isInClip()) )
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

void ViewProviderDrawingPage::unsetEdit(int ModNum)
{
    if (!view){
        Gui::Document* doc = Gui::Application::Instance->getDocument
            (pcObject->getDocument());
        view = new DrawingView(this, doc, Gui::getMainWindow());
        view->setWindowIcon(Gui::BitmapFactory().pixmap("actions/drawing-landscape"));

//        const char* objname = pcObject->Label.getValue();
//        view->setObjectName(QString::fromUtf8(objname));
//        view->onRelabel(doc);
//        view->setDocumentObject(pcObject->getNameInDocument());
        view->setWindowTitle(QObject::tr("Drawing viewer") + QString::fromAscii("[*]"));
        view->updateDrawing();
        view->updateTemplate(true);
        Gui::getMainWindow()->addWindow(view);
        view->viewAll();
    } else {
        view->updateDrawing();
        view->updateTemplate(true);
    }

    return;
}


DrawingView* ViewProviderDrawingPage::getDrawingView()
{
    if (!view) {
        Base::Console().Log("INFO - ViewProviderDrawingPage::getDrawingView has no view!\n");
        return 0;
    } else {
        return view;
    }
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
            Gui::Document* doc = Gui::Application::Instance->getDocument(pcObject->getDocument());
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

void ViewProviderDrawingPage::onChanged(const App::Property *prop)
{
  if (prop == &(getPageObject()->Views)) {
        if(view) {
            view->updateDrawing();
        }
    } else if (prop == &(getPageObject()->Template)) {
       if(view) {
            view->updateTemplate();
        }
    }

    Gui::ViewProviderDocumentObject::onChanged(prop);
}

void ViewProviderDrawingPage::finishRestoring()
{
    if (view) {
        view->updateDrawing(true);
    }
    Gui::ViewProviderDocumentObject::finishRestoring();
}


Drawing::FeaturePage* ViewProviderDrawingPage::getPageObject() const
{
    return dynamic_cast<Drawing::FeaturePage*>(pcObject);
}

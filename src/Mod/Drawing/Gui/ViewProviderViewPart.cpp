/***************************************************************************
 *   Copyright (c) 2014 Luke Parry <l.parry@warwick.ac.uk>                 *
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
# ifdef FC_OS_WIN32
#  include <windows.h>
# endif
#endif

/// Here the FreeCAD includes sorted by Base,App,Gui......
#include <Base/Console.h>
#include <Base/Parameter.h>
#include <Base/Exception.h>
#include <Base/Sequencer.h>
#include <App/Application.h>
#include <App/Document.h>
#include <App/DocumentObject.h>
#include <Gui/SoFCSelection.h>
#include <Gui/Selection.h>

#include <Mod/Drawing/App/FeatureViewPart.h>
#include <Mod/Drawing/App/FeatureViewDimension.h>

#include<Mod/Drawing/App/FeaturePage.h>
#include "ViewProviderViewPart.h"

using namespace DrawingGui;

PROPERTY_SOURCE(DrawingGui::ViewProviderViewPart, DrawingGui::ViewProviderDrawingView)

//**************************************************************************
// Construction/Destruction

ViewProviderViewPart::ViewProviderViewPart()
{
    sPixmap = "Page";
}

ViewProviderViewPart::~ViewProviderViewPart()
{
}

void ViewProviderViewPart::attach(App::DocumentObject *pcFeat)
{
    // call parent attach method
    ViewProviderDocumentObject::attach(pcFeat);
}

void ViewProviderViewPart::setDisplayMode(const char* ModeName)
{
    ViewProviderDocumentObject::setDisplayMode(ModeName);
}

std::vector<std::string> ViewProviderViewPart::getDisplayModes(void) const
{
    // get the modes of the father
    std::vector<std::string> StrList = ViewProviderDocumentObject::getDisplayModes();

    return StrList;
}


std::vector<App::DocumentObject*> ViewProviderViewPart::claimChildren(void) const
{
    // Collect any child fields and put this in the CamFeature tree
    std::vector<App::DocumentObject*> temp;
    const std::vector<App::DocumentObject *> &views = getViewPart()->getInList();
    try {
      for(std::vector<App::DocumentObject *>::const_iterator it = views.begin(); it != views.end(); ++it) {
          if((*it)->getTypeId().isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId())) {
              Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(*it);
              const std::vector<App::DocumentObject *> &refs = dim->References.getValues();
              for(std::vector<App::DocumentObject *>::const_iterator it = refs.begin(); it != refs.end(); ++it) {
                  if(strcmp(getViewPart()->getNameInDocument(), (*it)->getNameInDocument()) == 0) {
                     temp.push_back(dim);
                  }
              }
          }
      }
      return temp;
    } catch (...) {
        std::vector<App::DocumentObject*> tmp;
        return tmp;
    }
}


void ViewProviderViewPart::updateData(const App::Property*)
{
}

Drawing::FeatureViewPart* ViewProviderViewPart::getViewPart() const
{
    return dynamic_cast<Drawing::FeatureViewPart*>(pcObject);
}

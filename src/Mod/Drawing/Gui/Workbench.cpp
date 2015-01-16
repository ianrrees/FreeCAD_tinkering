/***************************************************************************
 *   Copyright (c) 2005 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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


#include "PreCompiled.h"

#ifndef _PreComp_
# include <qobject.h>
#endif

#include "Workbench.h"
#include <Gui/MenuManager.h>
#include <Gui/ToolBarManager.h>

using namespace DrawingGui;

#if 0 // needed for Qt's lupdate utility
//    qApp->translate("Workbench", "Drawing");
    qApp->translate("Workbench", "Drawing Pages");
    qApp->translate("Workbench", "Drawing Views");
    qApp->translate("Workbench", "Drawing Dimensions");
#endif

/// @namespace DrawingGui @class Workbench
TYPESYSTEM_SOURCE(DrawingGui::Workbench, Gui::StdWorkbench)

Workbench::Workbench()
{
}

Workbench::~Workbench()
{
}

Gui::MenuItem* Workbench::setupMenuBar() const
{
    Gui::MenuItem* root = StdWorkbench::setupMenuBar();
    Gui::MenuItem* item = root->findItem("&Windows");

    Gui::MenuItem* draw = new Gui::MenuItem;
    root->insertItem(item, draw);
    draw->setCommand("Drawing");
    //*draw << "Drawing_Open";
    *draw << "Drawing_NewPage";
    *draw << "Drawing_NewView";
    *draw << "Drawing_OrthoViews";
    *draw << "Drawing_NewViewSection";
    *draw << "Drawing_Annotation";
    *draw << "Drawing_NewDimension";
    *draw << "Drawing_ExportPage";
    *draw << "Drawing_Symbol";
    *draw << "Separator";
    *draw << "Drawing_ProjectShape";

    return root;
}

Gui::ToolBarItem* Workbench::setupToolBars() const
{
    Gui::ToolBarItem* root = StdWorkbench::setupToolBars();
    Gui::ToolBarItem* pages = new Gui::ToolBarItem(root);
    pages->setCommand("Drawing Pages");
    *pages << "Drawing_NewPage";

    Gui::ToolBarItem *views = new Gui::ToolBarItem(root);
    views->setCommand("Drawing Views");
    *views << "Drawing_NewView";
    *views << "Drawing_OrthoViews";
    *views << "Drawing_NewViewSection";
    *views << "Drawing_Annotation";

    Gui::ToolBarItem *dims = new Gui::ToolBarItem(root);
    dims->setCommand("Drawing Dimensions");
//    *dims << "Drawing_NewDimension"
    *dims << "Drawing_NewLengthDimension";
    *dims << "Drawing_NewDistanceXDimension";
    *dims << "Drawing_NewDistanceYDimension";
    *dims << "Drawing_NewRadiusDimension";
    *dims << "Drawing_NewDiameterDimension";
    *dims << "Drawing_NewAngleDimension";

    Gui::ToolBarItem *file = new Gui::ToolBarItem(root);
    file->setCommand("Drawing File Access");
    *file << "Drawing_ExportPage";
    *file << "Drawing_Symbol";
    return root;
}

Gui::ToolBarItem* Workbench::setupCommandBars() const
{
    Gui::ToolBarItem* root = new Gui::ToolBarItem;
    Gui::ToolBarItem *pages = new Gui::ToolBarItem(root);
    pages->setCommand("Drawing Pages");
    *pages << "Drawing_NewPage";

    Gui::ToolBarItem *views = new Gui::ToolBarItem(root);
    views->setCommand("Views");
    *views << "Drawing_NewView";
    *views << "Drawing_OrthoViews";
    *views << "Drawing_NewViewSection";
    *views << "Drawing_Annotation";

    Gui::ToolBarItem *dims = new Gui::ToolBarItem(root);
    dims->setCommand("Drawing Dimensions");
//    *dims << "Drawing_NewDimension";
    *dims << "Drawing_NewLengthDimension";
    *dims << "Drawing_NewDistanceXDimension";
    *dims << "Drawing_NewDistanceYDimension";
    *dims << "Drawing_NewRadiusDimension";
    *dims << "Drawing_NewDiameterDimension";
    *dims << "Drawing_NewAngleDimension";

    Gui::ToolBarItem *file = new Gui::ToolBarItem(root);
    file->setCommand("Drawing File Access");
    *file << "Drawing_ExportPage";
    *file << "Drawing_Symbol";

//     *img << "Drawing_OpenBrowserView";
//     *img << "Drawing_Clip";
//     *img << "Drawing_DraftView";

    return root;
}


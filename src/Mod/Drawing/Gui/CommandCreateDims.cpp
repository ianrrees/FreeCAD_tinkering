/***************************************************************************
 *   Copyright (c) 2014 Luke Parry <l.parry@warwick.ac.uk>                 *
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
# include <QMessageBox>
#include <QScopedPointer>
#endif

# include <App/DocumentObject.h>
# include <Gui/Action.h>
# include <Gui/Application.h>
# include <Gui/BitmapFactory.h>
# include <Gui/Command.h>
# include <Gui/Control.h>
# include <Gui/Document.h>
# include <Gui/Selection.h>
# include <Gui/MainWindow.h>
# include <Gui/FileDialog.h>
# include <Gui/ViewProvider.h>

# include <Mod/Part/App/PartFeature.h>

# include <Mod/Drawing/App/FeatureViewPart.h>
# include <Mod/Drawing/App/FeatureOrthoView.h>
# include <Mod/Drawing/App/FeatureViewOrthographic.h>
# include <Mod/Drawing/App/FeatureViewDimension.h>
# include <Mod/Drawing/App/FeaturePage.h>

# include "DrawingView.h"
# include "TaskDialog.h"
//# include "TaskOrthoViews.h"
# include "ViewProviderPage.h"

using namespace DrawingGui;
using namespace std;


//===========================================================================
// Drawing_NewDimension
//===========================================================================

DEF_STD_CMD(CmdDrawingNewDimension);

CmdDrawingNewDimension::CmdDrawingNewDimension()
  : Command("Drawing_NewDimension")
{
    sAppModule      = "Drawing";
    sGroup          = QT_TR_NOOP("Drawing");
    sMenuText       = QT_TR_NOOP("Insert a dimension into the drawing");
    sToolTipText    = QT_TR_NOOP("Insert a new dimension feature into the drawing");
    sWhatsThis      = "Drawing_NewDimension";
    sStatusTip      = sToolTipText;
    sPixmap         = "actions/drawing-annotation";
}

void CmdDrawingNewDimension::activated(int iMsg)
{

      // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    if(selection.size() == 0 || !selection[0].getObject()){
        return;
    }

    Drawing::FeatureViewPart * Obj = dynamic_cast<Drawing::FeatureViewPart *>(selection[0].getObject());

    if(!Obj) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No Feature View"),
                                                    QObject::tr("No FeatureView associated with this selection"));
        return;
    }

    App::DocumentObject *docObj = Obj->Source.getValue();

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();

    if (SubNames.size() > 2){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                   QObject::tr("Too many edges/vertices selected"));
        return;
    }

    std::vector<App::DocumentObject*> pages = this->getDocument()->getObjectsOfType(Drawing::FeaturePage::getClassTypeId());
    if (pages.empty()){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No Drawing Page"),
                                                   QObject::tr("Create a page first."));
        return;
    }
    Drawing::FeatureViewDimension *dim = 0;
    std::string support = selection[0].getAsPropertyLinkSubString();
    std::string FeatName = getUniqueObjectName("Dimension");

    std::string dimType;

    openCommand("Create Dimension");
    doCommand(Doc,"App.activeDocument().addObject('Drawing::FeatureViewDimension','%s')",FeatName.c_str());

    if (SubNames.size() == 1) {
        // Selected edge constraint
        if (SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge") {
            int GeoId = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());

            QScopedPointer<DrawingGeometry::BaseGeom> geom(Obj->getCompleteEdge(GeoId));

            dimType = "Distance";

            if(geom->geomType == DrawingGeometry::CIRCLE ||
               geom->geomType == DrawingGeometry::ARCOFCIRCLE ||
               geom->geomType == DrawingGeometry::ELLIPSE ||
               geom->geomType == DrawingGeometry::ARCOFELLIPSE ) {

                if(geom->geomType == DrawingGeometry::CIRCLE ) {
                    // Add center lines automatically for full circles
                    doCommand(Doc,"App.activeDocument().%s.CentreLines = True", FeatName.c_str());
                }

                // Radius Constraint
                dimType = "Radius";
            }

            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
            dim->References.setValue(Obj, SubNames[0].c_str());
        } else {
            abortCommand();
            return;
        }

    } else if(SubNames.size() == 2) {
        if (SubNames[0].size() > 6 && SubNames[0].substr(0,6) == "Vertex" &&
            SubNames[1].size() > 6 && SubNames[1].substr(0,6) == "Vertex") {
            int GeoId1 = std::atoi(SubNames[0].substr(6,std::string::npos).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(6,std::string::npos).c_str());

            dimType = "Distance";
            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);
            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);

        } else if(SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge" &&
                  SubNames[1].size() > 4 && SubNames[1].substr(0,4) == "Edge") {
            int GeoId1 = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(4,std::string::npos).c_str());

            // Project the edges
            Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart * >(Obj);

            QScopedPointer<DrawingGeometry::BaseGeom> ed1(Obj->getCompleteEdge(GeoId1));
            QScopedPointer<DrawingGeometry::BaseGeom> ed2(Obj->getCompleteEdge(GeoId2));

            if(ed1->geomType == DrawingGeometry::GENERIC &&
               ed2->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(ed1.data());
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(ed2.data());
                if(gen1->points.size() > 2 || gen2->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only straight line edges"));
                    abortCommand();
                    return;
                }

                // Construct edges
                Base::Vector2D lin1 = gen1->points.at(1) - gen1->points.at(0);
                Base::Vector2D lin2 = gen2->points.at(1) - gen2->points.at(0);

                // Cross product
                double xprod = fabs(lin1.fX * lin2.fY - lin1.fY * lin2.fX);

                //Base::Console().Log("xprod: %f\n", xprod);
                if(xprod < FLT_EPSILON) {
                    if(fabs(lin1.fX) < FLT_EPSILON && fabs(lin2.fX) < FLT_EPSILON)
                        dimType = "DistanceX";
                    else if(fabs(lin1.fY) < FLT_EPSILON && fabs(lin2.fY) < FLT_EPSILON)
                        dimType = "DistanceY";
                    else
                        dimType = "Distance";
                } else {
                    // Angle Measurement
                    dimType = "Angle";
                }

            } else {
                // Only support straight line edges
                QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                           QObject::tr("Only straight line edges can be currently supported"));
                abortCommand();
                return;
            }

            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);
            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);

        } else {

            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                       QObject::tr("Selection is not an Edge"));
            abortCommand();
            return;
        }
    }

    QString contentStr = QString::fromAscii("%value");

    if(strcmp(dimType.c_str(), "Angle") == 0) {
        // Append the degree symbol sign using unicode to content string
        contentStr += QString(QChar(0x00b0));

    } else if(strcmp(dimType.c_str(), "Radius") == 0) {
        contentStr.prepend(QString::fromAscii("r"));
    } else if(strcmp(dimType.c_str(), "Diameter") == 0) {
        contentStr += QString::fromAscii("D");
    }


    doCommand(Doc,"App.activeDocument().%s.Content = '%s'",FeatName.c_str()
                                                          ,contentStr.toStdString().c_str());

    // Check if the part is an orthographic view;
    Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(Obj);
    if(orthoView) {
        // Set the dimension to projected type
        doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
        dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    }

    dim->execute();
//     App::DocumentObject *dimObj = this->getDocument()->addObject("Drawing::FeatureViewDimension", getUniqueObjectName("Dimension").c_str());
//     Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(dimObj);

//     doCommand(Doc,"App.activeDocument().%s.References = %s",FeatName.c_str(), support.c_str());


    Drawing::FeaturePage *page = dynamic_cast<Drawing::FeaturePage *>(pages.front());
    page->addView(page->getDocument()->getObject(FeatName.c_str()));

    commitCommand();

    Obj->touch();
}

//===========================================================================
// Drawing_NewRadiusDimension
//===========================================================================

DEF_STD_CMD(CmdDrawingNewRadiusDimension);

CmdDrawingNewRadiusDimension::CmdDrawingNewRadiusDimension()
  : Command("Drawing_NewRadiusDimension")
{
    sAppModule      = "Drawing";
    sGroup          = QT_TR_NOOP("Drawing");
    sMenuText       = QT_TR_NOOP("Insert a new radius dimension into the drawing");
    sToolTipText    = QT_TR_NOOP("Insert a new radius dimension feature for the selected view");
    sWhatsThis      = "Drawing_NewRadiusDimension";
    sStatusTip      = sToolTipText;
    sPixmap         = "Dimension_Radius";
}

void CmdDrawingNewRadiusDimension::activated(int iMsg)
{

      // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    if(selection.size() == 0 || !selection[0].getObject()){
        return;
    }

    Drawing::FeatureViewPart * obj = dynamic_cast<Drawing::FeatureViewPart *>(selection[0].getObject());

    if(!obj) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                             QObject::tr("No Drawing Feature View for this selection"));
                             return;
    }

    App::DocumentObject *docObj = obj->Source.getValue();

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();

    if (SubNames.size() > 1){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
            QObject::tr("Too many items selected"));
        return;
    }

    std::vector<App::DocumentObject*> pages = this->getDocument()->getObjectsOfType(Drawing::FeaturePage::getClassTypeId());
    if (pages.empty()){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No Drawing Page"),
                                                   QObject::tr("Create a page first."));
        return;
    }

    Drawing::FeatureViewDimension *dim = 0;
    std::string support = selection[0].getAsPropertyLinkSubString();
    std::string FeatName = getUniqueObjectName("Dimension");

    std::string dimType;

    openCommand("Create Radius Dimension");

    doCommand(Doc,"App.activeDocument().addObject('Drawing::FeatureViewDimension','%s')",FeatName.c_str());

    // Get geometry of selected edge
    QScopedPointer<DrawingGeometry::BaseGeom> geom;
    if (SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge") {
        int GeoId = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());
        geom.reset(obj->getCompleteEdge(GeoId));
    } else {
        Base::Console().Error("Error - CmdDrawingNewRadiusDimension - Can't resolve selected edge - Dim: %s View: %s Source: %s Edge: %s**\n",
                            obj->getNameInDocument(),FeatName.c_str(),docObj->getNameInDocument(),SubNames[0].c_str());
    }

    if(geom->geomType == DrawingGeometry::CIRCLE ||
       geom->geomType == DrawingGeometry::ARCOFCIRCLE ||
       geom->geomType == DrawingGeometry::ELLIPSE ||
       geom->geomType == DrawingGeometry::ARCOFELLIPSE ) {
        dimType = "Radius";
        if(geom->geomType == DrawingGeometry::CIRCLE ||
           geom->geomType == DrawingGeometry::ELLIPSE ) {
            // Add center lines automatically for full circles
            doCommand(Doc,"App.activeDocument().%s.CentreLines = True", FeatName.c_str());
        }
        doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                           ,dimType.c_str());
        dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
        dim->References.setValue(obj, SubNames[0].c_str());
    } else {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                  QObject::tr("Edge selected was not of a circular type"));
        abortCommand();
        return;
    }

    QString contentStr = QString::fromAscii("r%value");

    doCommand(Doc,"App.activeDocument().%s.Content = '%s'",FeatName.c_str()
                                                          ,contentStr.toStdString().c_str());

    // Check if the part is an orthographic view;
    Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(obj);

    if(orthoView) {
        // Set the dimension to projected type
        doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
        dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    }

    dim->execute();
//     App::DocumentObject *dimObj = this->getDocument()->addObject("Drawing::FeatureViewDimension", getUniqueObjectName("Dimension").c_str());
//     Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(dimObj);

//     doCommand(Doc,"App.activeDocument().%s.References = %s",FeatName.c_str(), support.c_str());

    Drawing::FeaturePage *page = dynamic_cast<Drawing::FeaturePage *>(pages.front());
    page->addView(page->getDocument()->getObject(FeatName.c_str()));


    commitCommand();
}

//===========================================================================
// Drawing_NewDiameterDimension
//===========================================================================

DEF_STD_CMD(CmdDrawingNewDiameterDimension);

CmdDrawingNewDiameterDimension::CmdDrawingNewDiameterDimension()
  : Command("Drawing_NewDiameterDimension")
{
    sAppModule      = "Drawing";
    sGroup          = QT_TR_NOOP("Drawing");
    sMenuText       = QT_TR_NOOP("Insert a new diameter dimension into the drawing");
    sToolTipText    = QT_TR_NOOP("Insert a new diameter dimension feature for the selected view");
    sWhatsThis      = "Drawing_NewDiameterDimension";
    sStatusTip      = sToolTipText;
    sPixmap         = "Dimension_Diameter";
}

void CmdDrawingNewDiameterDimension::activated(int iMsg)
{

      // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    if(selection.size() == 0 || !selection[0].getObject()){
        return;
    }

    Drawing::FeatureViewPart * Obj = dynamic_cast<Drawing::FeatureViewPart *>(selection[0].getObject());

    if(!Obj) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
                             QObject::tr("Incorrect selection"));
                             return;
    }

    App::DocumentObject *docObj = Obj->Source.getValue();

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();

    if (SubNames.size() != 1 && SubNames.size() != 2){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
            QObject::tr("Incorrect selection"));
        return;
    }

    std::vector<App::DocumentObject*> pages = this->getDocument()->getObjectsOfType(Drawing::FeaturePage::getClassTypeId());
    if (pages.empty()){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No page to insert"),
            QObject::tr("Create a page to insert."));
        return;
    }

    Drawing::FeatureViewDimension *dim = 0;
    std::string support = selection[0].getAsPropertyLinkSubString();
    std::string FeatName = getUniqueObjectName("Dimension");

    std::string dimType;

    openCommand("Create Dimension");
    doCommand(Doc,"App.activeDocument().addObject('Drawing::FeatureViewDimension','%s')",FeatName.c_str());

    if (SubNames.size() == 1) {
        // Selected edge constraint
        if (SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge") {
            int GeoId = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());

            QScopedPointer<DrawingGeometry::BaseGeom> geom(Obj->getCompleteEdge(GeoId));

            dimType = "Distance";

            if(geom->geomType == DrawingGeometry::CIRCLE ||
               geom->geomType == DrawingGeometry::ARCOFCIRCLE ||
               geom->geomType == DrawingGeometry::ELLIPSE ||
               geom->geomType == DrawingGeometry::ARCOFELLIPSE ) {
                // Radius Constraint
                dimType = "Diameter";
            }

            if(geom->geomType == DrawingGeometry::CIRCLE ||
               geom->geomType == DrawingGeometry::ELLIPSE ) {
                // Add center lines automatically for full circles
                doCommand(Doc,"App.activeDocument().%s.CentreLines = True", FeatName.c_str());
            }

            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
            dim->References.setValue(Obj, SubNames[0].c_str());
        } else {
            abortCommand();
            return;
        }

    } else {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Please provide a valid selection"),
        QObject::tr("Incorrect selection"));
        abortCommand();
        return;
    }

    QString contentStr = QString::fromAscii("D%value");

    doCommand(Doc,"App.activeDocument().%s.Content = '%s'",FeatName.c_str()
                                                          ,contentStr.toStdString().c_str());

    // Check if the part is an orthographic view;
    Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(Obj);

    if(orthoView) {
        // Set the dimension to projected type
        doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
        dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    }

    dim->execute();
//     App::DocumentObject *dimObj = this->getDocument()->addObject("Drawing::FeatureViewDimension", getUniqueObjectName("Dimension").c_str());
//     Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(dimObj);

//     doCommand(Doc,"App.activeDocument().%s.References = %s",FeatName.c_str(), support.c_str());


    Drawing::FeaturePage *page = dynamic_cast<Drawing::FeaturePage *>(pages.front());
    page->addView(page->getDocument()->getObject(FeatName.c_str()));


    commitCommand();
}


//===========================================================================
// Drawing_NewLengthDimension
//===========================================================================

DEF_STD_CMD(CmdDrawingNewLengthDimension);

CmdDrawingNewLengthDimension::CmdDrawingNewLengthDimension()
  : Command("Drawing_NewLengthDimension")
{
    sAppModule      = "Drawing";
    sGroup          = QT_TR_NOOP("Drawing");
    sMenuText       = QT_TR_NOOP("Insert a new length dimension into the drawing");
    sToolTipText    = QT_TR_NOOP("Insert a new length dimension feature for the selected view");
    sWhatsThis      = "Drawing_NewLengthDimension";
    sStatusTip      = sToolTipText;
    sPixmap         = "Dimension_Length";
}

void CmdDrawingNewLengthDimension::activated(int iMsg)
{
    // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    if(selection.size() == 0 || !selection[0].getObject()){
        return;
    }

    Drawing::FeatureViewPart * Obj = dynamic_cast<Drawing::FeatureViewPart *>(selection[0].getObject());

    if(!Obj) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
                             QObject::tr("Incorrect selection"));
                             return;
    }

    App::DocumentObject *docObj = Obj->Source.getValue();

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();

    if (SubNames.size() != 1 && SubNames.size() != 2){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
            QObject::tr("Incorrect selection"));
        return;
    }

    std::vector<App::DocumentObject*> pages = this->getDocument()->getObjectsOfType(Drawing::FeaturePage::getClassTypeId());
    if (pages.empty()){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No page to insert"),
            QObject::tr("Create a page to insert."));
        return;
    }
    Drawing::FeatureViewDimension *dim = 0;
    std::string support = selection[0].getAsPropertyLinkSubString();
    std::string FeatName = getUniqueObjectName("Dimension");

    std::string dimType;

    openCommand("Create Dimension");
    doCommand(Doc,"App.activeDocument().addObject('Drawing::FeatureViewDimension','%s')",FeatName.c_str());

    if (SubNames.size() == 1) {
        // Selected edge constraint
        if (SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge") {
            int GeoId = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());

            QScopedPointer<DrawingGeometry::BaseGeom> geom(Obj->getCompleteEdge(GeoId));

            if(geom && geom->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(geom.data());
                if(gen1->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only one straight line edge"));
                    abortCommand();
                    return;
                }

                dimType = "Distance";
                doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                                  ,dimType.c_str());

                dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
                dim->References.setValue(Obj, SubNames[0].c_str());

            } else {
               // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only one straight line edge"));
                    abortCommand();
                    return;
            }
        } else {

            // Invalid selection has been made for one reference
            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                       QObject::tr("Please select a valid edge"));
            abortCommand();
            return;
        }
    } else if(SubNames.size() == 2) {
        if (SubNames[0].size() > 6 && SubNames[0].substr(0,6) == "Vertex" &&
            SubNames[1].size() > 6 && SubNames[1].substr(0,6) == "Vertex") {
            int GeoId1 = std::atoi(SubNames[0].substr(6,4000).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(6,4000).c_str());

            dimType = "Distance";
            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));

            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);

            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);

            dim->References.setValues(objs, subs);

        } else if(SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge" &&
                  SubNames[1].size() > 4 && SubNames[1].substr(0,4) == "Edge") {
            int GeoId1 = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(4,std::string::npos).c_str());

            // Project the edges
            Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart * >(Obj);
            QScopedPointer<DrawingGeometry::BaseGeom> ed1(Obj->getCompleteEdge(GeoId1));
            QScopedPointer<DrawingGeometry::BaseGeom> ed2(Obj->getCompleteEdge(GeoId2));


            if(ed1->geomType == DrawingGeometry::GENERIC &&
               ed2->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(ed1.data());
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(ed2.data());
                if(gen1->points.size() > 2 || gen2->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only straight line edges"));
                    abortCommand();
                    return;
                }

                dimType = "Distance";

            } else {
                // Only support straight line edges
                QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                           QObject::tr("Please provide a valid selection: Only straight line edges are allowed"));
                abortCommand();
                return;
            }

            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));

            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);

            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);


        } else {

            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Please provide a valid selection"),
                                                       QObject::tr("Incorrect selection"));
            abortCommand();
            return;
        }
    } else {

        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                   QObject::tr("Please select atleast two references"));
        abortCommand();
        return;
    }

    QString contentStr = QString::fromAscii("%value");

    doCommand(Doc,"App.activeDocument().%s.Content = '%s'",FeatName.c_str()
                                                          ,contentStr.toStdString().c_str());

    // Check if the part is an orthographic view;
    Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(Obj);
    if(orthoView) {
        // Set the dimension to projected type
        doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
        dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    }

    dim->execute();
//     App::DocumentObject *dimObj = this->getDocument()->addObject("Drawing::FeatureViewDimension", getUniqueObjectName("Dimension").c_str());
//     Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(dimObj);

//     doCommand(Doc,"App.activeDocument().%s.References = %s",FeatName.c_str(), support.c_str());


    Drawing::FeaturePage *page = dynamic_cast<Drawing::FeaturePage *>(pages.front());
    page->addView(page->getDocument()->getObject(FeatName.c_str()));


    commitCommand();
}

//===========================================================================
// Drawing_NewDistanceXDimension
//===========================================================================

DEF_STD_CMD(CmdDrawingNewDistanceXDimension);

CmdDrawingNewDistanceXDimension::CmdDrawingNewDistanceXDimension()
  : Command("Drawing_NewDistanceXDimension")
{
    sAppModule      = "Drawing";
    sGroup          = QT_TR_NOOP("Drawing");
    sMenuText       = QT_TR_NOOP("Insert a new x-distance dimension into the drawing");
    sToolTipText    = QT_TR_NOOP("Insert a new x-distance dimension feature for the selected view");
    sWhatsThis      = "Drawing_NewDistanceXDimension";
    sStatusTip      = sToolTipText;
    sPixmap         = "Dimension_Horizontal";
}

void CmdDrawingNewDistanceXDimension::activated(int iMsg)
{
    // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    if(selection.size() == 0 || !selection[0].getObject()){
        return;
    }

    Drawing::FeatureViewPart * Obj = dynamic_cast<Drawing::FeatureViewPart *>(selection[0].getObject());

    if(!Obj) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
                             QObject::tr("Incorrect selection"));
                             return;
    }

    App::DocumentObject *docObj = Obj->Source.getValue();

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();

    if (SubNames.size() != 1 && SubNames.size() != 2){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
            QObject::tr("Incorrect selection"));
        return;
    }

    std::vector<App::DocumentObject*> pages = this->getDocument()->getObjectsOfType(Drawing::FeaturePage::getClassTypeId());
    if (pages.empty()){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No page to insert"),
            QObject::tr("Create a page to insert."));
        return;
    }
    Drawing::FeatureViewDimension *dim = 0;
    std::string support = selection[0].getAsPropertyLinkSubString();
    std::string FeatName = getUniqueObjectName("Dimension");

    std::string dimType;

    openCommand("Create Dimension");
    doCommand(Doc,"App.activeDocument().addObject('Drawing::FeatureViewDimension','%s')",FeatName.c_str());

    if (SubNames.size() == 1) {
        // Selected edge constraint
        if (SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge") {
            int GeoId = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());

            QScopedPointer<DrawingGeometry::BaseGeom> geom(Obj->getCompleteEdge(GeoId));

            if(geom->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(geom.data());
                if(gen1->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only one straight line edge"));
                    abortCommand();
                    return;
                }

                Base::Vector2D lin1 = gen1->points.at(1) - gen1->points.at(0);

                if(fabs(lin1.fX) > FLT_EPSILON ) {
                    dimType = "DistanceX";
                } else {
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Invalid Selection"),
                                                               QObject::tr("Please select a non-vertical line"));
                    abortCommand();
                    return;
                }

                doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                                  ,dimType.c_str());

                dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
                dim->References.setValue(Obj, SubNames[0].c_str());

            } else {
               // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only one straight line edge"));
                    abortCommand();
                    return;
            }
        } else {

            // Invalid selection has been made for one reference
            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                       QObject::tr("Please select an edge"));
            abortCommand();
            return;
        }

    } else if(SubNames.size() == 2) {
        if (SubNames[0].size() > 6 && SubNames[0].substr(0,6) == "Vertex" &&
            SubNames[1].size() > 6 && SubNames[1].substr(0,6) == "Vertex") {
            int GeoId1 = std::atoi(SubNames[0].substr(6,4000).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(6,4000).c_str());

            dimType = "DistanceX";
            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);
            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);

        } else if(SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge" &&
                  SubNames[1].size() > 4 && SubNames[1].substr(0,4) == "Edge") {
            int GeoId1 = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(4,std::string::npos).c_str());

            // Project the edges
            Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart * >(Obj);
            QScopedPointer<DrawingGeometry::BaseGeom> ed1(Obj->getCompleteEdge(GeoId1));
            QScopedPointer<DrawingGeometry::BaseGeom> ed2(Obj->getCompleteEdge(GeoId2));

            if(ed1->geomType == DrawingGeometry::GENERIC &&
               ed2->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(ed1.data());
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(ed2.data());
                if(gen1->points.size() > 2 || gen2->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only straight line edges"));
                    abortCommand();
                    return;
                }

                 // Construct edges
                Base::Vector2D lin1 = gen1->points.at(1) - gen1->points.at(0);
                Base::Vector2D lin2 = gen2->points.at(1) - gen2->points.at(0);

                // Cross product
                double xprod = fabs(lin1.fX * lin2.fY - lin1.fY * lin2.fX);

                Base::Console().Log("xprod: %f\n", xprod);
                if(xprod > FLT_EPSILON) {
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Invalid Selection"),
                                                               QObject::tr("Please select parallel lines"));
                    abortCommand();
                    return;
                }

                if(fabs(lin1.fX) < FLT_EPSILON && fabs(lin2.fX) < FLT_EPSILON) {
                    dimType = "DistanceX";
                } else {
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Invalid Selection"),
                                                               QObject::tr("Please select vertical lines only"));
                    abortCommand();
                    return;
                }

            } else {
                // Only support straight line edges
                QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                           QObject::tr("Please provide a valid selection: Only straight line edges are allowed"));
                abortCommand();
                return;
            }

            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));

            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);

            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);

        } else {

            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                       QObject::tr("Please provide a valid selection"));
            abortCommand();
            return;
        }
    } else {

        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                    QObject::tr("Please provide a valid selection"));
        abortCommand();
        return;
    }

    QString contentStr = QString::fromAscii("%value");

    doCommand(Doc,"App.activeDocument().%s.Content = '%s'",FeatName.c_str()
                                                          ,contentStr.toStdString().c_str());

    // Check if the part is an orthographic view;
    Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(Obj);
    if(orthoView) {
        // Set the dimension to projected type
        doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
        dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    }

    dim->execute();
//     App::DocumentObject *dimObj = this->getDocument()->addObject("Drawing::FeatureViewDimension", getUniqueObjectName("Dimension").c_str());
//     Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(dimObj);

//     doCommand(Doc,"App.activeDocument().%s.References = %s",FeatName.c_str(), support.c_str());


    Drawing::FeaturePage *page = dynamic_cast<Drawing::FeaturePage *>(pages.front());
    page->addView(page->getDocument()->getObject(FeatName.c_str()));


    commitCommand();
}


//===========================================================================
// Drawing_NewDistanceYDimension
//===========================================================================

DEF_STD_CMD(CmdDrawingNewDistanceYDimension);

CmdDrawingNewDistanceYDimension::CmdDrawingNewDistanceYDimension()
  : Command("Drawing_NewDistanceYDimension")
{
    sAppModule      = "Drawing";
    sGroup          = QT_TR_NOOP("Drawing");
    sMenuText       = QT_TR_NOOP("Insert a new y-distance dimension into the drawing");
    sToolTipText    = QT_TR_NOOP("Insert a new y-distance dimension feature for the selected view");
    sWhatsThis      = "Drawing_NewDistanceYDimension";
    sStatusTip      = sToolTipText;
    sPixmap         = "Dimension_Vertical";
}

void CmdDrawingNewDistanceYDimension::activated(int iMsg)
{
    // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    if(selection.size() == 0 || !selection[0].getObject()){
        return;
    }

    Drawing::FeatureViewPart * Obj = dynamic_cast<Drawing::FeatureViewPart *>(selection[0].getObject());

    if(!Obj) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
                             QObject::tr("Incorrect selection"));
                             return;
    }

    App::DocumentObject *docObj = Obj->Source.getValue();

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();

    if (SubNames.size() != 1 && SubNames.size() != 2){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Wrong sized selection"),
            QObject::tr("Incorrect selection"));
        return;
    }

    std::vector<App::DocumentObject*> pages = this->getDocument()->getObjectsOfType(Drawing::FeaturePage::getClassTypeId());
    if (pages.empty()){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No page to insert"),
            QObject::tr("Create a page to insert."));
        return;
    }
    Drawing::FeatureViewDimension *dim = 0;
    std::string support = selection[0].getAsPropertyLinkSubString();
    std::string FeatName = getUniqueObjectName("Dimension");

    std::string dimType;

    openCommand("Create Dimension");
    doCommand(Doc,"App.activeDocument().addObject('Drawing::FeatureViewDimension','%s')",FeatName.c_str());

    if (SubNames.size() == 1) {
        // Selected edge constraint
        if (SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge") {
            int GeoId = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());

            QScopedPointer<DrawingGeometry::BaseGeom> geom(Obj->getCompleteEdge(GeoId));

            if(geom->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(geom.data());
                if(gen1->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only one straight line edge"));
                    abortCommand();
                    return;
                }

                // Construct edges
                Base::Vector2D lin1 = gen1->points.at(1) - gen1->points.at(0);

                if(fabs(lin1.fY) > FLT_EPSILON) {
                    dimType = "DistanceY";
                } else {
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Invalid Selection"),
                                                               QObject::tr("Please select a non-horizontal line"));
                    abortCommand();
                    return;
                }

                doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                                  ,dimType.c_str());

                dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
                dim->References.setValue(Obj, SubNames[0].c_str());

            } else {
               // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only one straight line edge"));
                    abortCommand();
                    return;
            }
        } else {

            // Invalid selection has been made for one reference
            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                       QObject::tr("Please select an edge"));
            abortCommand();
            return;
        }
    } else if(SubNames.size() == 2) {
        if (SubNames[0].size() > 6 && SubNames[0].substr(0,6) == "Vertex" &&
            SubNames[1].size() > 6 && SubNames[1].substr(0,6) == "Vertex") {
            int GeoId1 = std::atoi(SubNames[0].substr(6,4000).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(6,4000).c_str());

            dimType = "DistanceY";
            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));
            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);
            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);

        } else if(SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge" &&
                  SubNames[1].size() > 4 && SubNames[1].substr(0,4) == "Edge") {
            int GeoId1 = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(4,std::string::npos).c_str());

            // Project the edges
            Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart * >(Obj);
            QScopedPointer<DrawingGeometry::BaseGeom> ed1(Obj->getCompleteEdge(GeoId1));
            QScopedPointer<DrawingGeometry::BaseGeom> ed2(Obj->getCompleteEdge(GeoId2));

            if(ed1->geomType == DrawingGeometry::GENERIC &&
               ed2->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(ed1.data());
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(ed2.data());
                if(gen1->points.size() > 2 || gen2->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Please select only straight line edges"));
                    abortCommand();
                    return;
                }

                 // Construct edges
                Base::Vector2D lin1 = gen1->points.at(1) - gen1->points.at(0);
                Base::Vector2D lin2 = gen2->points.at(1) - gen2->points.at(0);

                // Cross product
                double xprod = fabs(lin1.fX * lin2.fY - lin1.fY * lin2.fX);

                Base::Console().Log("xprod: %f\n", xprod);
                if(xprod > FLT_EPSILON) {
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Invalid Selection"),
                                                               QObject::tr("Please select parallel lines"));
                    abortCommand();
                    return;
                }

                if(fabs(lin1.fY) < FLT_EPSILON && fabs(lin2.fY) < FLT_EPSILON) {
                    dimType = "DistanceY";
                } else {
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Invalid Selection"),
                                                               QObject::tr("Please select horizonal lines only"));
                    abortCommand();
                    return;
                }

            } else {
                // Only support straight line edges
                QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Please provide a valid selection: Only straight line edges are allowed"),
                                                           QObject::tr("Incorrect selection"));
                abortCommand();
                return;
            }

            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));

            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);

            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);

        } else {

            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Please provide a valid selection"),
            QObject::tr("Incorrect selection"));
            abortCommand();
            return;
        }
    } else {

        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Please select atleast two references"),
        QObject::tr("Incorrect selection"));
        abortCommand();
        return;
    }

    QString contentStr = QString::fromAscii("%value");

    doCommand(Doc,"App.activeDocument().%s.Content = '%s'",FeatName.c_str()
                                                          ,contentStr.toStdString().c_str());

    // Check if the part is an orthographic view;
    Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(Obj);
    if(orthoView) {
        // Set the dimension to projected type
        doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
        dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    }

    dim->execute();
//     App::DocumentObject *dimObj = this->getDocument()->addObject("Drawing::FeatureViewDimension", getUniqueObjectName("Dimension").c_str());
//     Drawing::FeatureViewDimension *dim = dynamic_cast<Drawing::FeatureViewDimension *>(dimObj);

//     doCommand(Doc,"App.activeDocument().%s.References = %s",FeatName.c_str(), support.c_str());


    Drawing::FeaturePage *page = dynamic_cast<Drawing::FeaturePage *>(pages.front());
    page->addView(page->getDocument()->getObject(FeatName.c_str()));


    commitCommand();
}


//===========================================================================
// Drawing_NewAngleDimension
//===========================================================================

DEF_STD_CMD(CmdDrawingNewAngleDimension);

CmdDrawingNewAngleDimension::CmdDrawingNewAngleDimension()
  : Command("Drawing_NewAngleDimension")
{
    sAppModule      = "Drawing";
    sGroup          = QT_TR_NOOP("Drawing");
    sMenuText       = QT_TR_NOOP("Insert a new angle dimension into the drawing");
    sToolTipText    = QT_TR_NOOP("Insert a new angle dimension feature for the selected view");
    sWhatsThis      = "Drawing_NewAngleDimension";
    sStatusTip      = sToolTipText;
    sPixmap         = "Dimension_Angle";
}

void CmdDrawingNewAngleDimension::activated(int iMsg)
{
    // get the selection
    std::vector<Gui::SelectionObject> selection = getSelection().getSelectionEx();
    if(selection.size() == 0 || !selection[0].getObject()){
        return;
    }

    Drawing::FeatureViewPart * Obj = dynamic_cast<Drawing::FeatureViewPart *>(selection[0].getObject());

    if(!Obj) {
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                             QObject::tr("No object in selection"));
                             return;
    }

    App::DocumentObject *docObj = Obj->Source.getValue();

    // get the needed lists and objects
    const std::vector<std::string> &SubNames = selection[0].getSubNames();

    if (SubNames.size() != 1 && SubNames.size() != 2){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
            QObject::tr("Select only 1 or 2 objects"));
        return;
    }

    std::vector<App::DocumentObject*> pages = this->getDocument()->getObjectsOfType(Drawing::FeaturePage::getClassTypeId());
    if (pages.empty()){
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("No Drawing Page"),
            QObject::tr("Create a page first."));
        return;
    }
    Drawing::FeatureViewDimension *dim = 0;
    std::string support = selection[0].getAsPropertyLinkSubString();
    std::string FeatName = getUniqueObjectName("Dimension");

    std::string dimType;

    openCommand("Create Dimension");
    doCommand(Doc,"App.activeDocument().addObject('Drawing::FeatureViewDimension','%s')",FeatName.c_str());

    if(SubNames.size() == 2) {
        if(SubNames[0].size() > 4 && SubNames[0].substr(0,4) == "Edge" &&
                  SubNames[1].size() > 4 && SubNames[1].substr(0,4) == "Edge") {
            int GeoId1 = std::atoi(SubNames[0].substr(4,std::string::npos).c_str());
            int GeoId2 = std::atoi(SubNames[1].substr(4,std::string::npos).c_str());

            // Project the edges
            Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart * >(Obj);
            QScopedPointer<DrawingGeometry::BaseGeom> ed1(Obj->getCompleteEdge(GeoId1));
            QScopedPointer<DrawingGeometry::BaseGeom> ed2(Obj->getCompleteEdge(GeoId2));

            if(ed1->geomType == DrawingGeometry::GENERIC &&
               ed2->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(ed1.data());
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(ed2.data());
                if(gen1->points.size() > 2 || gen2->points.size() > 2) {
                    // Only support straight line edges
                    QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                               QObject::tr("Selected object has too many points"));
                    abortCommand();
                    return;
                }

                dimType = "Angle";

            } else {
                // Only support straight line edges
                QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                                           QObject::tr("At least 1 selected object is not a straight line"));
                abortCommand();
                return;
            }

            doCommand(Doc,"App.activeDocument().%s.Type = '%s'",FeatName.c_str()
                                                               ,dimType.c_str());

            dim = dynamic_cast<Drawing::FeatureViewDimension *>(this->getDocument()->getObject(FeatName.c_str()));

            std::vector<App::DocumentObject *> objs;
            objs.push_back(Obj);
            objs.push_back(Obj);

            std::vector<std::string> subs;
            subs.push_back(SubNames[0]);
            subs.push_back(SubNames[1]);
            dim->References.setValues(objs, subs);                     //TODO: does References cause Dimension to become child of ViewPart?

        } else {
            //at least 1 object in selection is not an "Edge"
            QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                                 QObject::tr("Please select two edges"));
            abortCommand();
            return;
        }
    } else {
        //too many objects in selection
        QMessageBox::warning(Gui::getMainWindow(), QObject::tr("Incorrect selection"),
                             QObject::tr("Too many objects selected"));
        abortCommand();
        return;
    }

    QString contentStr = QString::fromAscii("%value");

    // Add an angle symbol using a unicode character
    contentStr += QString(QChar(0x00b0));
    doCommand(Doc,"App.activeDocument().%s.Content = '%s'",FeatName.c_str()
                                                          ,contentStr.toStdString().c_str());

    //TODO: "True" angles are not supported. see FeatureViewDimension.cpp::getDimvalue()
#if 0
    // Check if the part is an orthographic view;
    Drawing::FeatureOrthoView *orthoView = dynamic_cast<Drawing::FeatureOrthoView *>(Obj);
    if(orthoView) {
        // Set the dimension to projected type
        doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
        dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    }

    dim->execute();
#endif
    doCommand(Doc,"App.activeDocument().%s.ProjectionType = 'Projected'",FeatName.c_str());
    //dim->ProjectionType.StatusBits.set(2); // Set the projection type to read only
    dim->execute();

    //TODO: dimension sb child of FeatureView, not Page?
    Drawing::FeaturePage *page = dynamic_cast<Drawing::FeaturePage *>(pages.front());
    page->addView(page->getDocument()->getObject(FeatName.c_str()));



    commitCommand();
}

void CreateDrawingCommandsDims(void)
{
    Gui::CommandManager &rcCmdMgr = Gui::Application::Instance->commandManager();

    rcCmdMgr.addCommand(new CmdDrawingNewDimension());
    rcCmdMgr.addCommand(new CmdDrawingNewRadiusDimension());
    rcCmdMgr.addCommand(new CmdDrawingNewDiameterDimension());
    rcCmdMgr.addCommand(new CmdDrawingNewLengthDimension());
    rcCmdMgr.addCommand(new CmdDrawingNewDistanceXDimension());
    rcCmdMgr.addCommand(new CmdDrawingNewDistanceYDimension());
    rcCmdMgr.addCommand(new CmdDrawingNewAngleDimension());
}

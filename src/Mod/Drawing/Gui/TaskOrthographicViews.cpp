/***************************************************************************
 *   Copyright (c) 2014 Joe Dowsett <dowsettjoe[at]yahoo[dot]co[dot]uk>    *
 *   Copyright (c) 2014  Luke Parry <l.parry@warwick.ac.uk>                *
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

#include <Base/Console.h>

#include <Gui/Application.h>
#include <Gui/BitmapFactory.h>
#include <Gui/Command.h>

#include <Mod/Part/App/PartFeature.h>

#include <Mod/Drawing/App/FeaturePage.h>
#include <Mod/Drawing/App/FeatureViewPart.h>

#include <Mod/Drawing/App/FeatureOrthoView.h>
#include <Mod/Drawing/App/FeatureViewOrthographic.h>

#include "TaskOrthographicViews.h"
#include "ui_TaskOrthographicViews.h"

using namespace Gui;
using namespace DrawingGui;

#if 0 // needed for Qt's lupdate utility
    qApp->translate("QObject", "Make axonometric...");
    qApp->translate("QObject", "Edit axonometric settings...");
    qApp->translate("QObject", "Make orthographic");
#endif


TaskOrthographicViews::TaskOrthographicViews(Drawing::FeatureViewOrthographic* featView) : ui(new Ui_TaskOrthographicViews),
                                                                                           multiView(featView)
{
    ui->setupUi(this);

    blockUpdate = true;

    setFractionalScale(multiView->Scale.getValue());

    ui->cmbScaleType->setCurrentIndex(multiView->ScaleType.getValue());
    
    // Initially toggle view checkboxes if needed
    setupViewCheckbox(ui->chkOrthoLeft);
    setupViewCheckbox(ui->chkOrthoRight);
    setupViewCheckbox(ui->chkOrthoFront);
    setupViewCheckbox(ui->chkOrthoRear);
    setupViewCheckbox(ui->chkOrthoTop);
    setupViewCheckbox(ui->chkOrthoBottom);
    setupViewCheckbox(ui->chkIsoFrontTopLeft);
    setupViewCheckbox(ui->chkIsoFrontTopRight);
    setupViewCheckbox(ui->chkIsoFrontBottomRight);
    setupViewCheckbox(ui->chkIsoFrontBottomLeft);

    blockUpdate = false;

    // Rotation buttons
    // Note we don't do the custom one here, as it's handled by [a different function that's held up in customs]
    connect(ui->butTopRotate,   SIGNAL(clicked()), this, SLOT(rotateButtonClicked(void)));
    connect(ui->butCWRotate,    SIGNAL(clicked()), this, SLOT(rotateButtonClicked(void)));
    connect(ui->butRightRotate, SIGNAL(clicked()), this, SLOT(rotateButtonClicked(void)));
    connect(ui->butDownRotate,  SIGNAL(clicked()), this, SLOT(rotateButtonClicked(void)));
    connect(ui->butLeftRotate,  SIGNAL(clicked()), this, SLOT(rotateButtonClicked(void)));
    connect(ui->butCCWRotate,   SIGNAL(clicked()), this, SLOT(rotateButtonClicked(void)));

    // Orthographic check boxes
    connect(ui->chkOrthoLeft,   SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Left View
    connect(ui->chkOrthoRight,  SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Right View
    connect(ui->chkOrthoTop,    SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Top View
    connect(ui->chkOrthoBottom, SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Bottom View
    connect(ui->chkOrthoFront,  SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Front View
    connect(ui->chkOrthoRear,   SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Rear View

    // Front isometric check boxes
    connect(ui->chkIsoFrontTopLeft,     SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));
    connect(ui->chkIsoFrontTopRight,    SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));
    connect(ui->chkIsoFrontBottomRight, SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));
    connect(ui->chkIsoFrontBottomLeft,  SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));

    // Slot for Scale Type
    connect(ui->cmbScaleType, SIGNAL(currentIndexChanged(int)), this, SLOT(scaleTypeChanged(int)));
    connect(ui->scaleNum,     SIGNAL(textEdited(const QString &)), this, SLOT(scaleManuallyChanged(const QString &)));
    connect(ui->scaleDenom,   SIGNAL(textEdited(const QString &)), this, SLOT(scaleManuallyChanged(const QString &)));

    // Slot for Projection Type (layout)
    connect(ui->projection, SIGNAL(currentIndexChanged(int)), this, SLOT(projectionTypeChanged(int)));
}

TaskOrthographicViews::~TaskOrthographicViews()
{
    delete ui;
}

void TaskOrthographicViews::viewToggled(bool toggle)
{
    // Obtain name of checkbox
    QString viewName = sender()->objectName();

    //Gui::Command::openCommand("Toggle orthographic view");    //TODO: Is this for undo?

    if ( viewName.startsWith(QString::fromLatin1("chkOrtho")) ) {
        viewName = viewName.mid(8); // remove chkOrtho
    } else if ( viewName.startsWith(QString::fromLatin1("chkIso")) ) {
        viewName = viewName.mid(6); // remove chkIso
    } else {
        return;
    }

    if ( toggle && !multiView->hasOrthoView( viewName.toLatin1() ) ) {
        multiView->addOrthoView( viewName.toLatin1() );
    } else if ( !toggle && multiView->hasOrthoView( viewName.toLatin1() ) ) {
        multiView->removeOrthoView( viewName.toLatin1() );
    }

    /// Called to notify the GUI that the scale has changed
    Gui::Command::commitCommand();
    Gui::Command::updateActive();
}

void TaskOrthographicViews::rotateButtonClicked(void)
{
    if ( multiView && ui ) {
        const QObject *clicked = sender();

        // Any translation/scale/etc applied here will be ignored, as
        // FeatureViewOrthographic::setFrontViewOrientation() only
        // uses it to set Direction and XAxisDirection.
        Base::Matrix4D m = multiView->viewOrientationMatrix.getValue();

        // TODO: Construct these directly
        Base::Matrix4D t;

        //TODO: Consider changing the vectors around depending on whether we're in First or Third angle mode - might be more intuitive? IR
        if ( clicked == ui->butTopRotate ) {
            t.rotX(M_PI / -2);
        } else if ( clicked == ui->butCWRotate ) {
            t.rotY(M_PI / -2);
        } else if ( clicked == ui->butRightRotate) {
            t.rotZ(M_PI / 2);
        } else if ( clicked == ui->butDownRotate) {
            t.rotX(M_PI / 2);
        } else if ( clicked == ui->butLeftRotate) {
            t.rotZ(M_PI / -2);
        } else if ( clicked == ui->butCCWRotate) {
            t.rotY(M_PI / 2);
        }
        m *= t;

        multiView->setFrontViewOrientation(m);
        Gui::Command::updateActive();
    }
}

void TaskOrthographicViews::projectionTypeChanged(int index)
{
    if(blockUpdate)
        return;

    Gui::Command::openCommand("Update orthographic projection type");
    if(index == 0) {
        //layout per Page (Document)
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ProjectionType = '%s'", multiView->getNameInDocument()
                                                                                             , "Document");
    } else if(index == 1) {
        // First Angle layout
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ProjectionType = '%s'", multiView->getNameInDocument()
                                                                                             , "First Angle");
    } else if(index == 2) {
        // Third Angle layout
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ProjectionType = '%s'", multiView->getNameInDocument()
                                                                                             , "Third Angle");
    } else {
        Gui::Command::abortCommand();
        Base::Console().Log("Error - TaskOrthographicViews::projectionTypeChanged - unknown projection layout: %d\n",index);
        return;
    }
    // need to recalculate the positions of the views here (at least top/bottom and left/right)
    Gui::Command::commitCommand();
    Gui::Command::updateActive();
}

void TaskOrthographicViews::scaleTypeChanged(int index)
{
    if(blockUpdate)
        return;

    Gui::Command::openCommand("Update orthographic scale type");
    if(index == 0) {
        //Automatic Scale Type
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", multiView->getNameInDocument()
                                                                                             , "Document");
    } else if(index == 1) {
        // Document Scale Type
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", multiView->getNameInDocument()
                                                                                             , "Automatic");
    } else if(index == 2) {
        // Custom Scale Type
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", multiView->getNameInDocument()
                                                                                             , "Custom");
    } else {
        Gui::Command::abortCommand();
        Base::Console().Log("Error - TaskOrthographicViews::scaleTypeChanged - unknown scale type: %d\n",index);
        return;
    }
    Gui::Command::commitCommand();
    Gui::Command::updateActive();
}

// ** David Eppstein / UC Irvine / 8 Aug 1993
// Reworked 2015 IR to add the power of logarithms!
void TaskOrthographicViews::nearestFraction(double val, int &n, int &d) const
{
    int exponent = std::floor(std::log10(val));
    if (exponent > 1 || exponent < -1) {
        val *= std::pow(10, -exponent);
    }

    n = 1;  // numerator
    d = 1;  // denominator
    double fraction = n / d;
    double m = std::abs(fraction - val);

    while (std::abs(fraction - val) > 0.001) {
        if (fraction < val) {
            ++n;
        } else {
            ++d;
            n = (int) std::round(val * d);
        }
        fraction = n / (double) d;
    }

    if (exponent > 1) {
            n *= std::pow(10, exponent);
    } else if (exponent < -1) {
            d *= std::pow(10, -exponent);
    }
}

void TaskOrthographicViews::updateTask()
{
    // Update the scale type
    this->blockUpdate = true;
    ui->cmbScaleType->setCurrentIndex(multiView->ScaleType.getValue());

    // Update the scale value
    setFractionalScale(multiView->Scale.getValue());

    this->blockUpdate = false;
}


void TaskOrthographicViews::setFractionalScale(double newScale)
{
    blockUpdate = true;
    int num, den;

    nearestFraction(newScale, num, den);

    ui->scaleNum->setText(QString::number(num));
    ui->scaleDenom->setText(QString::number(den));
    blockUpdate = false;
}

void TaskOrthographicViews::scaleManuallyChanged(const QString & text)
{
    //TODO: See what this is about - shouldn't be simplifying the scale ratio while it's being edited... IR
    if(blockUpdate)
        return;

    bool ok1, ok2;

    int a = ui->scaleNum->text().toInt(&ok1);
    int b = ui->scaleDenom->text().toInt(&ok2);

    double scale = (double) a / (double) b;
    if (ok1 && ok2) {
        // If we were not in Custom, switch to Custom in two steps
        bool switchToCustom = (strcmp(multiView->ScaleType.getValueAsString(), "Custom") != 0);
        if(switchToCustom) {
            // First, send out command to put us into custom scale
            scaleTypeChanged(ui->cmbScaleType->findText(QString::fromLatin1("Custom")));
            switchToCustom = true;
        }

        Gui::Command::openCommand("Update custom scale");
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.Scale = %f", multiView->getNameInDocument()
                                                                                         , scale);
        Gui::Command::commitCommand();
        Gui::Command::updateActive();

        if(switchToCustom) {
            // Second, update the GUI
            ui->cmbScaleType->setCurrentIndex(ui->cmbScaleType->findText(QString::fromLatin1("Custom")));
        }
    }
}

void TaskOrthographicViews::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void TaskOrthographicViews::setupViewCheckbox(QCheckBox *box)
{
    QString viewName = box->objectName();

    if ( viewName.startsWith(QString::fromLatin1("chkOrtho")) &&
         multiView->hasOrthoView(viewName.mid(8).toLatin1()) ) {
            box->setCheckState(Qt::Checked);
    } else if ( viewName.startsWith(QString::fromLatin1("chkIso")) &&
                multiView->hasOrthoView(viewName.mid(6).toLatin1())) {
            box->setCheckState(Qt::Checked);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TODO: Do we really need to hang on to the TaskDlgOrthographicViews in this class? IR
TaskDlgOrthographicViews::TaskDlgOrthographicViews(Drawing::FeatureViewOrthographic* featView) : TaskDialog(), 
                                                                                                 multiView(featView)
{
    orthographicView = dynamic_cast<const ViewProviderViewOrthographic *>(featView);
    widget  = new TaskOrthographicViews(featView);
    taskbox = new Gui::TaskView::TaskBox(Gui::BitmapFactory().pixmap("actions/drawing-orthoviews"),
                                         widget->windowTitle(), true, 0);
    taskbox->groupLayout()->addWidget(widget);
    Content.push_back(taskbox);
}

TaskDlgOrthographicViews::~TaskDlgOrthographicViews()
{
}

void TaskDlgOrthographicViews::update()
{
    widget->updateTask();
}

//==== calls from the TaskView ===============================================================
void TaskDlgOrthographicViews::open()
{
}

void TaskDlgOrthographicViews::clicked(int)
{
}

bool TaskDlgOrthographicViews::accept()
{
    return true;//!widget->user_input();
}

bool TaskDlgOrthographicViews::reject()
{
    return true;
}


#include "moc_TaskOrthographicViews.cpp"


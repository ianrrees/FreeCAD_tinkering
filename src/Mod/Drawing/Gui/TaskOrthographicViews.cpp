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

    int a,b;
    double scale = multiView->Scale.getValue();

    nearestFraction(scale, a,b,10);

    //nearestFraction(scale, a, b, 10);
    ui->scaleNum->setText(QString::number(a));
    ui->scaleDenom->setText(QString::number(b));
    ui->cmbScaleType->setCurrentIndex(multiView->ScaleType.getValue());
    

    if(strcmp(multiView->ScaleType.getValueAsString(), "Automatic") == 0 ||
       strcmp(multiView->ScaleType.getValueAsString(), "Document") == 0) {
        ui->scaleNum->setDisabled(true);
        ui->scaleDenom->setDisabled(true);
    }

    // Initially toggle checkboxes if needed
    if(multiView->hasOrthoView("Left")) {
        ui->chkOrthoLeft->setCheckState(Qt::Checked);
    }

    if(multiView->hasOrthoView("Right")) {
        ui->chkOrthoRight->setCheckState(Qt::Checked);
    }

    if(multiView->hasOrthoView("Front")) {
        ui->chkOrthoFront->setCheckState(Qt::Checked);
    }

    if(multiView->hasOrthoView("Rear")) {
        ui->chkOrthoRear->setCheckState(Qt::Checked);
    }

    if(multiView->hasOrthoView("Top")) {
        ui->chkOrthoTop->setCheckState(Qt::Checked);
    }

    if(multiView->hasOrthoView("Bottom")) {
        ui->chkOrthoBottom->setCheckState(Qt::Checked);
    }

    blockUpdate = false;

    // Connect the checkboxes to their views
    connect(ui->chkOrthoLeft,   SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Left View
    connect(ui->chkOrthoRight,  SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Right View
    connect(ui->chkOrthoTop,    SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Top View
    connect(ui->chkOrthoBottom, SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Bottom View
    connect(ui->chkOrthoFront,  SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Front View
    connect(ui->chkOrthoRear,   SIGNAL(toggled(bool)), this, SLOT(viewToggled(bool)));    // Rear View

    // Slot for Scale Type
    connect(ui->cmbScaleType, SIGNAL(currentIndexChanged(int)), this, SLOT(scaleTypeChanged(int)));
    connect(ui->scaleNum,     SIGNAL(textEdited(const QString &)), this, SLOT(scaleChanged(const QString &)));
    connect(ui->scaleDenom,   SIGNAL(textEdited(const QString &)), this, SLOT(scaleChanged(const QString &)));

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
    QString viewName = sender()->objectName().mid(8); // remove chkOrtho
    //Gui::Command::openCommand("Toggle orthographic view");

    if (toggle && !multiView->hasOrthoView(viewName.toLatin1())) {
        Drawing::FeatureOrthoView *view = dynamic_cast<Drawing::FeatureOrthoView *>(multiView->addOrthoView(viewName.toLatin1()));
    } else if(!toggle && multiView->hasOrthoView(viewName.toLatin1())) {
        multiView->removeOrthoView(viewName.toLatin1());
    }

    Gui::Command::commitCommand();
    Gui::Command::updateActive();
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
        ui->scaleDenom->setDisabled(true);
        ui->scaleNum->setDisabled(true);
    } else if(index == 1) {
        // Document Scale Type
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", multiView->getNameInDocument()
                                                                                             , "Automatic");
        ui->scaleDenom->setDisabled(true);
        ui->scaleNum->setDisabled(true);
    } else if(index == 2) {
        // Custom Scale Type
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", multiView->getNameInDocument()
                                                                                             , "Custom");
        ui->scaleDenom->setDisabled(false);
        ui->scaleNum->setDisabled(false);
    } else {
        Gui::Command::abortCommand();
        Base::Console().Log("Error - TaskOrthographicViews::scaleTypeChanged - unknown scale type: %d\n",index);
        return;
    }
    Gui::Command::commitCommand();
    Gui::Command::updateActive();
}

// ** David Eppstein / UC Irvine / 8 Aug 1993
void TaskOrthographicViews::nearestFraction(const double &val, int &n, int &d, const long &maxDenom) const
{
        n = 1;  // numerator
        d = 1;  // denominator
        double fraction = n / d;
        double m = std::abs(fraction - val);
        while (std::abs(fraction - val) > 0.001)
        {
            if (fraction < val)
            {
                n++;
            }
            else
            {
                d++;
                n = (int) round(val * d);
            }
            fraction = n / (double) d;
        }
}

void TaskOrthographicViews::updateTask()
{
    // Update the scale type
    this->blockUpdate = true;
    ui->cmbScaleType->setCurrentIndex(multiView->ScaleType.getValue());

    int a, b;
    // Update the scale value
    double scale = multiView->Scale.getValue();

    nearestFraction(scale, a,b,10);

    //nearestFraction(scale, a, b, 10);
    ui->scaleNum->setText(QString::number(a));
    ui->scaleDenom->setText(QString::number(b));

    this->blockUpdate = false;
}

void TaskOrthographicViews::scaleChanged(const QString & text)
{
    if(blockUpdate)
        return;

    bool ok1, ok2;

    int a = ui->scaleNum->text().toInt(&ok1);
    int b = ui->scaleDenom->text().toInt(&ok2);

    double scale = (double) a / (double) b;
    if (ok1 && ok2) {
        Gui::Command::openCommand("Update custom scale");
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.Scale = %f", multiView->getNameInDocument()
                                                                                         , scale);
        Gui::Command::commitCommand();
        Gui::Command::updateActive();
    }
}

void TaskOrthographicViews::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TaskDlgOrthographicViews::TaskDlgOrthographicViews(Drawing::FeatureViewOrthographic* featView) : TaskDialog(), 
                                                                                                 multiView(featView),
                                                                                                 orthographicView(0)
{
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


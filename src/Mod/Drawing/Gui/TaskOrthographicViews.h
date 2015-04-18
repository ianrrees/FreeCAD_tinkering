/***************************************************************************
 *   Copyright (c) 2011 Joe Dowsett <j-dowsett[at]users.sourceforge.net>   *
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

#ifndef GUI_TASKVIEW_TASKORTHOGRAPHICVIEWS_H
#define GUI_TASKVIEW_TASKORTHOGRAPHICVIEWS_H

#include <Base/BoundBox.h>
#include <Gui/TaskView/TaskView.h>
#include <Gui/TaskView/TaskDialog.h>

#include "ui_TaskOrthographicViews.h"

#include "ViewProviderViewOrthographic.h"
#include "../App/FeatureViewOrthographic.h"


class Ui_TaskOrthographicViews;

namespace App {
  class Document;
  class DocumentObject;
}

namespace Drawing {
  class FeatureViewPart;
  class FeatureViewOrthographic;
}

namespace DrawingGui
{

class TaskOrthographicViews : public QWidget
{
    Q_OBJECT

public:
    TaskOrthographicViews(Drawing::FeatureViewOrthographic* featView);
    ~TaskOrthographicViews();

public:
    void updateTask();
    void nearestFraction(const double &val, int &a, int &b, const long &maxDenom) const;
    /// Sets the numerator and denominator widgets to match newScale
    void setFractionalScale(double newScale);

protected Q_SLOTS:
    void viewToggled(bool toggle);

    /// Requests appropriate rotation of our FeatureViewOrthographic
    void rotateButtonClicked(void);

    void projectionTypeChanged(int index);
    void scaleTypeChanged(int index);
    void scaleManuallyChanged(const QString & text);

protected:
    void changeEvent(QEvent *e);

    /// Updates state of box to match the state of multiView
    void setupViewCheckbox(QCheckBox *box);

private:
    //class Private;
    Ui_TaskOrthographicViews * ui;
    bool blockUpdate;

protected:
  ViewProviderViewOrthographic *orthographicView;
  Drawing::FeatureViewOrthographic* multiView;
};

/// Simulation dialog for the TaskView
class TaskDlgOrthographicViews : public Gui::TaskView::TaskDialog
{
    Q_OBJECT

public:
    TaskDlgOrthographicViews(Drawing::FeatureViewOrthographic* featView);
    ~TaskDlgOrthographicViews();

    const ViewProviderViewOrthographic * getOrthographicView() const { return orthographicView; }
    Drawing::FeatureViewOrthographic * getMultiView() const { return multiView; }
public:
    /// is called the TaskView when the dialog is opened
    virtual void open();
    /// is called by the framework if an button is clicked which has no accept or reject role
    virtual void clicked(int);
    /// is called by the framework if the dialog is accepted (Ok)
    virtual bool accept();
    /// is called by the framework if the dialog is rejected (Cancel)
    virtual bool reject();
    /// is called by the framework if the user presses the help button
    virtual void helpRequested() { return;}
    virtual bool isAllowedAlterDocument(void) const
    { return false; }

    void update();

protected:
    const ViewProviderViewOrthographic *orthographicView;
    Drawing::FeatureViewOrthographic *multiView;
    
private:
    TaskOrthographicViews * widget;
    Gui::TaskView::TaskBox* taskbox;
};

} //namespace DrawingGui

#endif // GUI_TASKVIEW_OrthographicViews_H

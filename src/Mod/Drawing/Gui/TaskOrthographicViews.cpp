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


TaskOrthographicViews::TaskOrthographicViews(ViewProviderViewOrthographic *vp) : ui(new Ui_TaskOrthographicViews),
                                                                                 orthographicView(vp)
{
    ui->setupUi(this);

    blockUpdate = true;

    Drawing::FeatureViewOrthographic *orthoFeat = orthographicView->getObject();

    int a,b;
    double scale = orthoFeat->Scale.getValue();

    nearestFraction(scale, a,b,10);

    //nearestFraction(scale, a, b, 10);
    ui->scaleNum->setText(QString::number(a));
    ui->scaleDenom->setText(QString::number(b));
    ui->cmbScaleType->setCurrentIndex(orthoFeat->ScaleType.getValue());
    


    if(strcmp(orthoFeat->ScaleType.getValueAsString(), "Automatic") == 0 ||
       strcmp(orthoFeat->ScaleType.getValueAsString(), "Document") == 0) {
        ui->scaleNum->setDisabled(true);
        ui->scaleDenom->setDisabled(true);
    }

    // Initially toggle checkboxes if needed
    if(orthoFeat->hasOrthoView("Left")) {
        ui->chkOrthoLeft->setCheckState(Qt::Checked);
    }

    if(orthoFeat->hasOrthoView("Right")) {
        ui->chkOrthoRight->setCheckState(Qt::Checked);
    }

    if(orthoFeat->hasOrthoView("Front")) {
        ui->chkOrthoFront->setCheckState(Qt::Checked);
    }

    if(orthoFeat->hasOrthoView("Rear")) {
        ui->chkOrthoRear->setCheckState(Qt::Checked);
    }

    if(orthoFeat->hasOrthoView("Top")) {
        ui->chkOrthoTop->setCheckState(Qt::Checked);
    }

    if(orthoFeat->hasOrthoView("Bottom")) {
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

#if 0
    // **********************************************************************
    // note that checkboxes are numbered increasing right & down
    // while OrthoViews relative positions are increasing right & up
    // doh!  I should renumber the checkboxes for clarity
    // **********************************************************************

    //   [x+2][y+2]
    c_boxes[0][2] = ui->cb02;       //left most, x = -2, y = 0
    c_boxes[1][1] = ui->cb11;
    c_boxes[1][2] = ui->cb12;
    c_boxes[1][3] = ui->cb13;
    c_boxes[2][0] = ui->cb20;       //top most, x = 0, y = -2
    c_boxes[2][1] = ui->cb21;
    c_boxes[2][2] = ui->cb22;       //centre (primary view) checkbox x = y = 0.
    c_boxes[2][3] = ui->cb23;
    c_boxes[2][4] = ui->cb24;       //bottom most, x = 0, y = 2
    c_boxes[3][1] = ui->cb31;
    c_boxes[3][2] = ui->cb32;
    c_boxes[3][3] = ui->cb33;
    c_boxes[4][2] = ui->cb42;       //right most, x = 2, y = 0

    for (int i=0; i < 5; i++)
        for (int j=0; j < 5; j++)
            if ((abs(i-2) + abs(j-2)) < 3) {
                //if i,j combination corresponds to valid check box, then proceed with:
                connect(c_boxes[i][j], SIGNAL(toggled(bool)), this, SLOT(cb_toggled(bool)));
                connect(c_boxes[i][j], SIGNAL(customContextMenuRequested(const QPoint&)),this, SLOT(ShowContextMenu(const QPoint&)));
            }

    // access scale / position QLineEdits via array
    inputs[0] = ui->scale_0;
    inputs[1] = ui->x_1;
    inputs[2] = ui->y_2;
    inputs[3] = ui->spacing_h_3;
    inputs[4] = ui->spacing_v_4;

    for (int i=0; i < 5; i++)
    {
        connect(inputs[i],  SIGNAL(textEdited(const QString &)), this, SLOT(data_entered(const QString &)));
        connect(inputs[i],  SIGNAL(returnPressed()), this, SLOT(text_return()));
    }

    connect(ui->projection, SIGNAL(currentIndexChanged(int)), this, SLOT(projectionChanged(int)));
    connect(ui->smooth,     SIGNAL(stateChanged(int)), this, SLOT(smooth(int)));
    connect(ui->hidden,     SIGNAL(stateChanged(int)), this, SLOT(hidden(int)));
    connect(ui->auto_tog,   SIGNAL(stateChanged(int)), this, SLOT(toggle_auto(int)));

    connect(ui->view_from,  SIGNAL(currentIndexChanged(int)), this, SLOT(setPrimary(int)));
    connect(ui->axis_right, SIGNAL(currentIndexChanged(int)), this, SLOT(setPrimary(int)));

    connect(ui->axoProj,    SIGNAL(activated(int)), this, SLOT(change_axo(int)));
    connect(ui->axoUp,      SIGNAL(activated(int)), this, SLOT(change_axo(int)));
    connect(ui->axoRight,   SIGNAL(activated(int)), this, SLOT(change_axo(int)));
    connect(ui->vert_flip,  SIGNAL(clicked()),      this, SLOT(axo_button()));
    connect(ui->tri_flip,   SIGNAL(clicked()),      this, SLOT(axo_button()));
    connect(ui->axoScale,   SIGNAL(textEdited(const QString &)), this, SLOT(axo_scale(const QString &)));
    connect(ui->axoScale,   SIGNAL(returnPressed()), this, SLOT(text_return()));

    ui->tabWidget->setTabEnabled(1,false);
#endif
}

void TaskOrthographicViews::viewToggled(bool toggle)
{
    // Obtain name of checkbox
    QString viewName = sender()->objectName().mid(8); // remove chkOrtho

    Drawing::FeatureViewOrthographic *orthoFeat = orthographicView->getObject();

    Gui::Command::openCommand("Toggle orthographic view");

    double spacing = 25.; // stick with defualt 10 mm

    if (toggle && !orthoFeat->hasOrthoView(viewName.toLatin1())) {
        Drawing::FeatureOrthoView *view = dynamic_cast<Drawing::FeatureOrthoView *>(orthoFeat->addOrthoView(viewName.toLatin1()));
        Drawing::FeatureOrthoView *anchorView =  dynamic_cast<Drawing::FeatureOrthoView *>(orthoFeat->Anchor.getValue());

        view->execute();

        if(view && anchorView) {
            Base::BoundBox3d abbox = anchorView->getBoundingBox();
            Base::BoundBox3d vbbox = view->getBoundingBox();

            // Position the new view
            if(strcmp(view->Type.getValueAsString(), "Left") == 0) {
                view->X.setValue((vbbox.LengthX() + abbox.LengthX()) / -2. - spacing);
            } else if(strcmp(view->Type.getValueAsString(), "Right") == 0) {
                view->X.setValue((vbbox.LengthX() + abbox.LengthX()) /  2. + spacing);
            } else if(strcmp(view->Type.getValueAsString(), "Top") == 0) {
                view->Y.setValue((vbbox.LengthY() + abbox.LengthY()) /  2. + spacing);
            } else if(strcmp(view->Type.getValueAsString(), "Bottom") == 0) {
                view->Y.setValue((vbbox.LengthY() + abbox.LengthY()) / -2. - spacing);
            } else if(strcmp(view->Type.getValueAsString(), "Rear") == 0) {
                if(orthoFeat->hasOrthoView("Top")) {
                    Drawing::FeatureOrthoView *topView =  dynamic_cast<Drawing::FeatureOrthoView *>(orthoFeat->getOrthoView("Top"));
                    Base::BoundBox3d tbbox = topView->getBoundingBox();
                    view->Y.setValue((vbbox.LengthY() + tbbox.LengthY() + abbox.LengthY()) / 2. + 2. * spacing);
                } else {
                  view->Y.setValue((vbbox.LengthY() + abbox.LengthY()) / -2. - spacing); // Set in top position for now
                }

            }
        } else {
            Base::Console().Error("Couldn't add orthographic view");
        }


    } else if(!toggle){
        orthoFeat->removeOrthoView(viewName.toLatin1());
    }

    Gui::Command::commitCommand();
    Gui::Command::updateActive();

}

void TaskOrthographicViews::scaleTypeChanged(int index)
{

    if(blockUpdate)
        return;

    Drawing::FeatureViewOrthographic *orthoFeat = orthographicView->getObject();
    Gui::Command::openCommand("Update orthographic scale type");
    if(index == 0) {
        //Automatic Scale Type
        ui->scaleDenom->setDisabled(true);
        ui->scaleNum->setDisabled(true);

        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", orthoFeat->getNameInDocument()
                                                                                             , "Document");



    } else if(index == 1) {
        // Document Scale Type
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", orthoFeat->getNameInDocument()
                                                                                             , "Automatic");

        ui->scaleDenom->setDisabled(true);
        ui->scaleNum->setDisabled(true);

    } else if(index == 2) {

        // Custom Scale Type
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.ScaleType = '%s'", orthoFeat->getNameInDocument()
                                                                                             , "Custom");

        ui->scaleDenom->setDisabled(false);
        ui->scaleNum->setDisabled(false);
    } else {
        Gui::Command::abortCommand();
        return;
    }

    Gui::Command::commitCommand();
    Gui::Command::updateActive();
}

// ** David Eppstein / UC Irvine / 8 Aug 1993
void TaskOrthographicViews::nearestFraction(const double &val, int &n, int &d, const long &maxDenom) const
{
//     long m[2][2];
//     double x, startx;
//     long ai;
//
//     startx = x = val;
//
//     /* initialize matrix */
//     m[0][0] = m[1][1] = 1;
//     m[0][1] = m[1][0] = 0;
//
//     /* loop finding terms until denom gets too big */
//     while (m[1][0] *  ( ai = (long)x ) + m[1][1] <= maxDenom) {
//         long t;
//         t = m[0][0] * ai + m[0][1];
//         m[0][1] = m[0][0];
//         m[0][0] = t;
//         t = m[1][0] * ai + m[1][1];
//         m[1][1] = m[1][0];
//         m[1][0] = t;
//         if(x==(double)ai) break;     // AF: division by zero
//         x = 1/(x - (double) ai);
//         if(x>(double)0x7FFFFFFF) break;  // AF: representation failure
//     }
//
//     /* now remaining x is between 0 and 1/ai */
//     /* approx as either 0 or 1/m where m is max that will fit in maxden */
//     /* first try zero */
// //     printf("%ld/%ld, error = %e\n", m[0][0], m[1][0],
// //            startx - ((double) m[0][0] / (double) m[1][0]));
//
//     /* now try other possibility */
//     ai = (maxDenom - m[1][1]) / m[1][0];
//     a = m[0][0] * ai + m[0][1];
//     b = m[1][0] * ai + m[1][1];

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

    Drawing::FeatureViewOrthographic *orthoFeat = orthographicView->getObject();

    // Update the scale type
    this->blockUpdate = true;
    ui->cmbScaleType->setCurrentIndex(orthoFeat->ScaleType.getValue());

    int a, b;
    // Update the scale value
    double scale = orthoFeat->Scale.getValue();

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
        Drawing::FeatureViewOrthographic *orthoFeat = orthographicView->getObject();
        Gui::Command::openCommand("Update custom scale");
        Gui::Command::doCommand(Gui::Command::Doc, "App.activeDocument().%s.Scale = %f", orthoFeat->getNameInDocument()
                                                                                         , scale);
        Gui::Command::commitCommand();
        Gui::Command::updateActive();
    }


}

TaskOrthographicViews::~TaskOrthographicViews()
{
    delete ui;
}

void TaskOrthographicViews::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}


TaskDlgOrthographicViews::TaskDlgOrthographicViews(ViewProviderViewOrthographic *vp) : TaskDialog(), orthographicView(vp)
{
    widget  = new TaskOrthographicViews(vp);
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

#if 0
void pagesize(const std::string &page_template, int dims[4], int block[4])
{
    dims[0] = 10;               // default to A4_Landscape with 10mm margins
    dims[1] = 10;
    dims[2] = 287;
    dims[3] = 200;

    block[0] = block[1] = 0;    // default to no title block
    block[2] = block[3] = 0;

    int t0, t1, t2, t3 = 0;

    //code below copied from FeaturePage.cpp
    Base::FileInfo fi(page_template);
    if (!fi.isReadable())
    {
        fi.setFile(App::Application::getResourceDir() + "Mod/Drawing/Templates/" + fi.fileName());
        if (!fi.isReadable())       //if so then really shouldn't have been able to get this far, but just in case...
            return;
    }

    // open Template file
    std::string line;
    ifstream file (fi.filePath().c_str());

    try
    {
        while (!file.eof())
        {
            getline (file,line);

            if (line.find("<!-- Working space") != std::string::npos)
            {
                sscanf(line.c_str(), "%*s %*s %*s %d %d %d %d", &dims[0], &dims[1], &dims[2], &dims[3]);        //eg "    <!-- Working space 10 10 410 287 -->"
                getline (file,line);

                if (line.find("<!-- Title block") != std::string::npos)
                    sscanf(line.c_str(), "%*s %*s %*s %d %d %d %d", &t0, &t1, &t2, &t3);    //eg "    <!-- Working space 10 10 410 287 -->"

                break;
            }

            if (line.find("metadata") != std::string::npos)      //give up if we meet a metadata tag
                break;
        }
    }
    catch (Standard_Failure)
    { }

    file.close();

    if (t3 != 0)
    {
        block[2] = t2 - t0;             // block width
        block[3] = t3 - t1;             // block height

        if (t0 <= dims[0])              // title block on left
            block[0] = -1;
        else if (t2 >= dims[2])         // title block on right
            block[0] = 1;

        if (t1 <= dims[1])              // title block at top
            block[1] = 1;
        else if (t3 >= dims[3])         // title block at bottom
            block[1] = -1;
    }

    dims[2] -= dims[0];         // width
    dims[3] -= dims[1];         // height
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////


Orthoview::Orthoview(App::DocumentObject *part, App::DocumentObject *page, Base::BoundBox3d *partbox)
{
    App::Document *doc = App::GetApplication().getActiveDocument();

    std::string myname = doc->getUniqueObjectName("orthoview");

    Base::Vector3d boxCenter = partbox->CalcCenter();
    centerCoord = Base::Vector3f(boxCenter.x, boxCenter.y, boxCenter.z);

    App::DocumentObject *docObj = doc->addObject("Drawing::FeatureViewPart", myname.c_str());

    if(!docObj)
        throw Base::Exception("Cannot create Drawing::FeatureViewPart");

    this->view = static_cast<Drawing::FeatureViewPart *>(docObj);
    static_cast<App::DocumentObjectGroup *>(page)->addObject(this->view);
    this->view->Source.setValue(part);

    scale = 1;

    rel_x = 0;
    rel_y = 0;
    ortho = true;
    auto_scale = true;
}


Orthoview::~Orthoview()
{
}

void Orthoview::setData(const int &r_x, const int &r_y)
{
    this->rel_x = r_x;
    this->rel_y = r_y;

    char label[15];
    std::sprintf(label, "Ortho_%i_%i", rel_x, rel_y);  // label name for view, based on relative position

    this->view->Label.setValue(label);
    ortho = ((rel_x * rel_y) == 0);
}


void Orthoview::remove()
{
    App::GetApplication().getActiveDocument()->remObject(this->view->getNameInDocument());
    this->view = 0; // Set as null pointer incase
}

void Orthoview::setPos(const float &px, const float &py)
{
//     if (px != 0 && py !=0){
//         pageX = px;
//         pageY = py;
//     }

    float x, y;
    calcCentre(x,y);

    float ox = px - scale * x,
          oy = py + scale * y;

    this->view->X.setValue(ox);
    this->view->Y.setValue(oy);
}


void Orthoview::setScale(const float &newScale)
{
    this->scale = newScale;
    this->view->Scale.setValue(scale);
}


float Orthoview::getScale() const
{
    return scale;
}


void Orthoview::calcCentre(float &x, float &y) const
{
    x = X_dir.X() * centerCoord.x + X_dir.Y() * centerCoord.y + X_dir.Z() * centerCoord.z;
    y = Y_dir.X() * centerCoord.x + Y_dir.Y() * centerCoord.y + Y_dir.Z() * centerCoord.z;
}

void Orthoview::showHidden(bool state)
{
    this->view->ShowHiddenLines.setValue(state);
}

void Orthoview::showSmooth(bool state)
{
    //throw Base::Exception("Redundant method");
}

void Orthoview::setProjection(const gp_Ax2 &cs)
{
    gp_Ax2  actual_cs;
    gp_Dir  actual_X;

    // coord system & directions for desired projection
    X_dir = cs.XDirection();
    Y_dir = cs.YDirection();
    Z_dir = cs.Direction();

    // coord system of created view - same code as used in projection algos
    actual_cs = gp_Ax2(gp_Pnt(0.,0.,0.), gp_Dir(Z_dir.X(),Z_dir.Y(),Z_dir.Z()));
    actual_X  = actual_cs.XDirection();

    // angle between desired projection and actual projection
    float rotation = X_dir.Angle(actual_X);

    if (rotation != 0 && std::abs(M_PI - rotation) > 0.05)
        if (!Z_dir.IsEqual(actual_X.Crossed(X_dir), 0.05))
            rotation = -rotation;

    this->view->Direction.setValue(Z_dir.X(), Z_dir.Y(), Z_dir.Z());
    this->view->Rotation.setValue(rotation * 180 / M_PI);
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

OrthoViews::OrthoViews(const char *pagename, const char *partname)
{
    page_name = pagename;
    part_name = partname;

    App::Document *doc = App::GetApplication().getActiveDocument();

    part = doc->getObject(partname);
    bbox.Add(static_cast<Part::Feature*>(part)->Shape.getBoundingBox());

    page = doc->getObject(pagename);
    load_page();

    min_space = 15;             // should be preferenced

    min_r_x = max_r_x = 0;
    min_r_y = max_r_y = 0;

    rotate_coeff = 1;
    smooth = false;
    hidden = false;
    autodims = true;
}


OrthoViews::~OrthoViews()
{

    for (std::vector<Orthoview *>::iterator it = views.begin(); it != views.end(); it++) {
        delete *it;
        (*it) = 0;
    }
    views.clear();

    page->recompute();
}

void OrthoViews::getPageSize(double &width, double &height) const
{
    Drawing::FeaturePage *featPage = static_cast<Drawing::FeaturePage*>(page);
    Drawing::FeatureTemplate *templ = dynamic_cast<Drawing::FeatureTemplate *>(featPage->Template.getValue());
    height = templ->getHeight();
    width  = templ->getWidth();
}

void OrthoViews::getMarginSize(double &marginX, double &marginY) const
{
    // TODO implement margin method for template
    marginX = 10;
    marginY = 10;
}

void OrthoViews::load_page()
{


    //pagesize(template_name, large, block);
    page_dims = large;

    double pgWidth, pgHeight;
    getPageSize(pgWidth, pgHeight);

    double marginWidth, marginHeight;
    getMarginSize(marginWidth, marginHeight);

//     // process page dims for title block data
//     if (block[0] != 0) {
//         title = true;
//
//         // max vertical space avoiding title block
//         small_v[1] = large[1];                  // y margin same as large page
//         small_v[3] = large[3];                  // y size same as large page
//         small_v[2] = large[2] - block[2];       // x width same as large width - block width
//         if (block[0] == -1) {
//             small_v[0] = large[0] + block[2];   // x margin same as large + block width
//             horiz = &min_r_x;
//         } else  {
//             small_v[0] = large[0];              // x margin same as large
//             horiz = &max_r_x;
//         }
//
//         // max horizontal space avoiding title block
//         small_h[0] = large[0];
//         small_h[2] = large[2];
//         small_h[3] = large[3] - block[3];
//         if (block[1] == 1) {
//             small_h[1] = large[1] + block[3];
//             vert = &max_r_y;
//         } else {
//             small_h[1] = large[1];
//             vert = &min_r_y;
//         }
//     } else
//         title = false;
}


void OrthoViews::calc_layout_size()                         // calculate the real world size of given view layout, assuming no space
{
    // note that views in relative positions x = -4, -2, 0 , 2 etc etc
    // have width = orientated part width
    // while those in relative positions x = -3, -1, 1 etc
    // have width = orientated part depth

    // similarly in y positions, height = part height or depth

    layout_width = (1 + floor(max_r_x / 2.0) + floor(-min_r_x / 2.0)) * width;
    layout_width += (ceil(max_r_x / 2.0) + ceil(-min_r_x / 2.0)) * depth;
    layout_height = (1 + floor(max_r_y / 2.0) + floor(-min_r_y / 2.0)) * height;
    layout_height += (ceil(max_r_y / 2.0) + ceil(-min_r_y / 2.0)) * depth;
}


void OrthoViews::choose_page()                              // chooses which bit of page space to use depending upon layout & titleblock
{
    int   h = abs(*horiz);                                                                  // how many views in direction of title block  (horiz points to min_r_x or max_r_x)
    int   v = abs(*vert);
    float layout_corner_width = (1 + floor(h / 2.0)) * width + ceil(h / 2.0) * depth;       // from (0, 0) view inclusively, how wide and tall is the layout in the direction of the title block
    float layout_corner_height = (1 + floor(v / 2.0)) * height + ceil(v / 2.0) * depth;
    float rel_space_x = layout_corner_width / layout_width - 1.0 * block[2] / large[2];     // relative to respective sizes, how much space between (0, 0) and title block,
    float rel_space_y = layout_corner_height / layout_height - 1.0 * block[3] / large[3];   //                      can be -ve if block extends into / beyond (0, 0) view
    float view_x, view_y, v_x_r, v_y_r;
    bool  interferes = false;
    float a, b;

    for (int i = min_r_x; i <= max_r_x; i++)
        for (int j = min_r_y; j <= max_r_y; j++)
            // is there a view in this position?
            if (index(i, j) != -1) {
                a = i * block[0] * 0.5;                                 // reflect i and j so as +ve is in direction of title block ##
                b = j * block[1] * 0.5;
                view_x = ceil(a + 0.5) * width + ceil(a) * depth;       // extreme coords of view in direction of block, measured from far corner of (0, 0) view,
                view_y = ceil(b + 0.5) * height + ceil(b) * depth;      //                      can be -ve if view is on opposite side of (0, 0) from title block
                v_x_r = view_x / layout_width;                          // make relative
                v_y_r = view_y / layout_height;
                if (v_x_r > rel_space_x && v_y_r > rel_space_y)         // ## so that can use > in this condition regardless of position of block
                    interferes = true;
            }

    if (!interferes)
        page_dims = large;
    else {
        if (std::min(small_h[2] / layout_width, small_h[3] / layout_height) > std::min(small_v[2] / layout_width, small_v[3] / layout_height))
            page_dims = small_h;
        else
            page_dims = small_v;
    }
}

// Keep
void OrthoViews::calc_scale()                               // compute scale required to meet minimum space requirements
{

    double pgWidth, pgHeight;
    this->getPageSize(pgWidth, pgHeight);

    float scale_x = ((float)pgWidth - (float) num_gaps_x * min_space) / layout_width;
    float scale_y = ((float)pgHeight- (float) num_gaps_y * min_space) / layout_height;

    float working_scale = std::min(scale_x, scale_y);

    //which gives the largest scale for which the min_space requirements can be met, but we want a 'sensible' scale, rather than 0.28457239...
    //eg if working_scale = 0.115, then we want to use 0.1, similarly 7.65 -> 5, and 76.5 -> 50

    float exponent = std::floor(std::log10(working_scale));                  //if working_scale = a * 10^b, what is b?
    working_scale *= std::pow(10, -exponent);                                //now find what 'a' is.

    float valid_scales[2][8] = {{1.0, 1.25, 2.0, 2.5, 3.75, 5.0, 7.5, 10.0},   //equate to 1:10, 1:8, 1:5, 1:4, 3:8, 1:2, 3:4, 1:1
                                {1.0, 1.5 , 2.0, 3.0, 4.0 , 5.0, 8.0, 10.0}};  //equate to 1:1, 3:2, 2:1, 3:1, 4:1, 5:1, 8:1, 10:1

    int i = 7;
    while (valid_scales[(exponent >= 0)][i] > working_scale)                 //choose closest value smaller than 'a' from list.
        i -= 1;                                                              //choosing top list if exponent -ve, bottom list for +ve exponent

    scale = valid_scales[(exponent >= 0)][i] * pow(10, exponent);            //now have the appropriate scale, reapply the *10^b
}


void OrthoViews::calc_offsets()                             // calcs SVG coords for centre of upper left view
{
    // space_x is the emptry clear white space between views
    // gap_x is the centre - centre distance between views

    double pgWidth, pgHeight;
    this->getPageSize(pgWidth, pgHeight);

    double marginWidth, marginHeight;
    this->getMarginSize(marginWidth, marginHeight);

    float space_x = ((float) pgWidth - scale * layout_width)  / (float) num_gaps_x;
    float space_y = ((float) pgHeight - scale * layout_height) / (float) num_gaps_y;

    gap_x = space_x + scale * (width + depth) * 0.5;
    gap_y = space_y + scale * (height + depth) * 0.5;

    if (min_r_x % 2 == 0)
        offset_x = marginWidth + space_x + 0.5 * scale * width;
    else
        offset_x = marginWidth + space_x + 0.5 * scale * depth;

    if (max_r_y % 2 == 0)
        offset_y = marginHeight + space_y + 0.5 * scale * height;
    else
        offset_y = marginHeight + space_y + 0.5 * scale * depth;
}


void OrthoViews::set_views()                                // process all views - scale & positions
{
    float x, y;

    for (std::vector<Orthoview *>::iterator it = views.begin(); it != views.end(); it++){
        x = offset_x + ((*it)->rel_x - min_r_x) * gap_x;
        y = offset_y + (max_r_y - (*it)->rel_y) * gap_y;

        if ((*it)->auto_scale) {
            (*it)->setScale(scale);
        }

        (*it)->setPos(x, y);
    }
}


void OrthoViews::process_views()                            // update scale and positions of views
{
    if (autodims) {
        calc_layout_size();

        if (title)
            choose_page();

        calc_scale();
        calc_offsets();
    }

    set_views();
    App::GetApplication().getActiveDocument()->recompute();
}

// keep
void OrthoViews::set_hidden(bool state)
{
    hidden = state;

    for (unsigned int i = 0; i < views.size(); i++)
        views[i]->showHidden(hidden);

    App::GetApplication().getActiveDocument()->recompute();
}

// redundant
void OrthoViews::set_smooth(bool state)
{
    smooth = state;

    for (std::vector<Orthoview *>::iterator it = views.begin(); it != views.end(); it++)
        (*it)->showSmooth(smooth);

    App::GetApplication().getActiveDocument()->recompute();
}


void OrthoViews::set_primary(gp_Dir facing, gp_Dir right)   // set the orientation of the primary view
{
    primary.SetDirection(facing);
    primary.SetXDirection(right);
    gp_Dir up = primary.YDirection();

    // compute dimensions of part when orientated according to primary view
    width  = std::abs(right.X()  * bbox.LengthX() + right.Y()  * bbox.LengthY() + right.Z()  * bbox.LengthZ());
    height = std::abs(up.X()     * bbox.LengthX() + up.Y()     * bbox.LengthY() + up.Z()     * bbox.LengthZ());
    depth  = std::abs(facing.X() * bbox.LengthX() + facing.Y() * bbox.LengthY() + facing.Z() * bbox.LengthZ());

    if (views.size() == 0)
        add_view(0, 0);
    else {
        views[0]->setProjection(primary);
        set_all_orientations();                 // reorient all other views appropriately
        process_views();
    }
}

void OrthoViews::set_orientation(int index)                 // set orientation of single view
{
    double  rotation;
    int     n;                                              // how many 90* rotations from primary view?
    gp_Dir  dir;                                            // rotate about primary x axis (if in a relative y position) or y axis?
    gp_Ax2  cs;

    if (views[index]->ortho) {
        if (views[index]->rel_x != 0) {
            dir = primary.YDirection();
            n = views[index]->rel_x;
        } else {
            dir = primary.XDirection();
            n = -views[index]->rel_y;
        }

        rotation = n * rotate_coeff * M_PI/2;              // rotate_coeff is -1 or 1 for 1st or 3rd angle
        cs = primary.Rotated(gp_Ax1(gp_Pnt(0,0,0), dir), rotation);
        views[index]->setProjection(cs);
    }
}

void OrthoViews::set_all_orientations()                     // set orientations of all views (ie projection or primary changed)
{
    // start from 1 - the 0 is the primary view
    for (unsigned int i = 1; i < views.size(); i++) {
        if (views[i]->ortho)
            set_orientation(i);
        else
            set_Axo(views[i]->rel_x, views[i]->rel_y);
    }
}

void OrthoViews::set_projection(int proj)                   // 1 = 1st angle, 3 = 3rd angle
{
    if (proj == 3)
        rotate_coeff = 1;
    else if (proj == 1)
        rotate_coeff = -1;

    set_all_orientations();
    process_views();
}


void OrthoViews::add_view(int rel_x, int rel_y)             // add a new view to the layout
{
    if (index(rel_x, rel_y) == -1) {
        Orthoview * view = new Orthoview(part, page, &bbox);
        view->setData(rel_x, rel_y);
        views.push_back(view);

        max_r_x = std::max(max_r_x, rel_x);
        min_r_x = std::min(min_r_x, rel_x);
        max_r_y = std::max(max_r_y, rel_y);
        min_r_y = std::min(min_r_y, rel_y);

        num_gaps_x = max_r_x - min_r_x + 2;
        num_gaps_y = max_r_y - min_r_y + 2;

        int i = views.size() - 1;
        views[i]->showHidden(hidden);
        views[i]->showSmooth(smooth);

        if (views[i]->ortho)
            set_orientation(i);
        else
            set_Axo(rel_x, rel_y);

        process_views();
    }
}


void OrthoViews::del_view(int rel_x, int rel_y)             // remove a view from the layout
{
    int num = index(rel_x, rel_y);

    if (num > 0) {
        views[num]->remove();
        delete views[num];
        views.erase(views.begin() + num);

        min_r_x = max_r_x = 0;
        min_r_y = max_r_y = 0;

        for (std::vector<Orthoview *>::const_iterator it = views.begin(); it != views.end(); it++){
            min_r_x = std::min(min_r_x, (*it)->rel_x);     // calculate extremes from remaining views
            max_r_x = std::max(max_r_x, (*it)->rel_x);
            min_r_y = std::min(min_r_y, (*it)->rel_y);
            max_r_y = std::max(max_r_y, (*it)->rel_y);
        }

        num_gaps_x = max_r_x - min_r_x + 2;
        num_gaps_y = max_r_y - min_r_y + 2;

        process_views();
    }
}


void OrthoViews::del_all()
{
    for (std::vector<Orthoview *>::iterator it = views.begin(); it != views.end(); it++){
        delete *it;
        *it = 0;
    }
    views.clear();
}


int OrthoViews::is_Ortho(int rel_x, int rel_y) const              // is the view at r_x, r_y an ortho or axo one?
{
    int result = index(rel_x, rel_y);

    if (result != -1)
        result = views[result]->ortho;

    return result;
}


int OrthoViews::index(int rel_x, int rel_y) const                // index in vector of view, -1 if doesn't exist
{
    int index = -1;

    int i = 0;
    for (std::vector<Orthoview *>::const_iterator it = views.begin(); it != views.end(); it++, i++){
        if ((*it)->rel_x == rel_x && (*it)->rel_y == rel_y) {
            index = i;
            break;
        }
    }
    return index;
}


void OrthoViews::set_Axo_scale(const int &rel_x, const int &rel_y, const float &axo_scale)       // set an axo scale independent of ortho ones
{
    int num = index(rel_x, rel_y);

    if (num != -1 && !views[num]->ortho){
        views[num]->auto_scale = false;
        views[num]->setScale(axo_scale);
        views[num]->setPos(0., 0.);
        App::GetApplication().getActiveDocument()->recompute();
    }
}

void OrthoViews::set_Axo(int rel_x, int rel_y, gp_Dir up, gp_Dir right, bool away, AxoMode axoMode, bool tri)   // set custom axonometric view
{
    int     num = index(rel_x, rel_y);
    double  rotations[2];
    gp_Dir  dir;

    views[num]->ortho = false;
    views[num]->away = away;
    views[num]->tri = tri;
    views[num]->axo = axoMode;

    if (axoMode == ISOMETRIC) {
        rotations[0] = -0.7853981633974476;
        rotations[1] = -0.6154797086703873;
    } else if (axoMode == DIMETRIC) {
        rotations[0] = -0.7853981633974476;
        rotations[1] = -0.2712637537260206;
    } else if(tri) {
        rotations[0] = -1.3088876392502007;
        rotations[1] = -0.6156624905260762;
    } else {
        rotations[0] = 1.3088876392502007 - M_PI/2;
        rotations[1] = -0.6156624905260762;
    }

    if (away)
        rotations[1] = - rotations[1];

    gp_Ax2  cs = gp_Ax2(gp_Pnt(0,0,0), right);
    cs.SetYDirection(up);
    cs.Rotate(gp_Ax1(gp_Pnt(0,0,0), up), rotations[0]);
    dir = cs.XDirection();
    cs.Rotate(gp_Ax1(gp_Pnt(0,0,0), dir), rotations[1]);

    views[num]->up = up;
    views[num]->right = right;
    views[num]->setProjection(cs);
    views[num]->setPos(0., 0.);

    App::GetApplication().getActiveDocument()->recompute();
}


void OrthoViews::set_Axo(int rel_x, int rel_y)              // set view to default axo projection
{
    int num = index(rel_x, rel_y);

    if (num != -1) {
        gp_Dir up = primary.YDirection();                   // default to view from up and right
        gp_Dir right = primary.XDirection();
        bool away = false;

        if (rel_x * rel_y != 0) {
            // but change default if it's a diagonal position
            if (rotate_coeff == 1) {
                 // third angle
                away = (rel_y < 0);

                if (rel_x < 0)
                    right = primary.Direction();
                else
                    right = primary.XDirection();
            } else {
                away = (rel_y > 0);

                if (rel_x > 0)
                    right = primary.Direction();
                else
                    right = primary.XDirection();
            }
        }
        set_Axo(rel_x, rel_y, up, right, away);
    }
}


void OrthoViews::set_Ortho(const int &rel_x, const int &rel_y)            // return view to orthographic
{
    int num = index(rel_x, rel_y);

    if (num != -1 && rel_x * rel_y == 0) {
        views[num]->ortho = true;
        views[num]->setScale(scale);
        views[num]->auto_scale = true;
        set_orientation(num);
        views[num]->setPos(0., 0.);

        App::GetApplication().getActiveDocument()->recompute();
    }
}


bool OrthoViews::get_Axo(int rel_x, int rel_y,
                         int &axo,
                         gp_Dir &up, gp_Dir &right,
                         bool &away,
                         bool &tri,
                         float &axo_scale)
{
    int num = index(rel_x, rel_y);

    if(num != -1 && !views[num]->ortho) {
        axo = views[num]->axo;
        up = views[num]->up;
        right = views[num]->right;
        away = views[num]->away;
        tri = views[num]->tri;
        axo_scale = views[num]->getScale();
        return true;
    } else
        return false;
}


void OrthoViews::auto_dims(bool setting)
{
    autodims = setting;
    if (autodims)
        process_views();
}


void OrthoViews::set_configs(float configs[5])              // for autodims off, set scale & positionings
{
    if (!autodims) {
        scale = configs[0];
        offset_x = configs[1];
        offset_y = configs[2];
        gap_x = configs[3];
        gap_y = configs[4];
        process_views();
    }
}


void OrthoViews::get_configs(float configs[5]) const          // get scale & positionings
{
    configs[0] = scale;
    configs[1] = offset_x;
    configs[2] = offset_y;
    configs[3] = gap_x;
    configs[4] = gap_y;
}

#endif


#if 0
void TaskOrthographicViews::ShowContextMenu(const QPoint& pos)
{
    QString name = sender()->objectName().right(2);
    char letter = name.toStdString()[0];
    int dx = letter - '0' - 2;

    letter = name.toStdString()[1];
    int dy = letter - '0' - 2;

    if (c_boxes[dx + 2][dy + 2]->isChecked()) {
        QString str_1 = QObject::tr("Make axonometric...");
        QString str_2 = QObject::tr("Edit axonometric settings...");
        QString str_3 = QObject::tr("Make orthographic");

        QPoint globalPos = c_boxes[dx + 2][dy + 2]->mapToGlobal(pos);
        QMenu myMenu;

        if (orthos->is_Ortho(dx, -dy))
            myMenu.addAction(str_1);
        else {
            myMenu.addAction(str_2);
            if (dx * dy == 0)
                myMenu.addAction(str_3);
        }

        QAction * selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            QString text = selectedItem->text();

            if (text == str_1) {
                 // make axo
                orthos->set_Axo(dx, -dy);
                axo_r_x = dx;
                axo_r_y = dy;
                ui->tabWidget->setTabEnabled(1, true);
                ui->tabWidget->setCurrentIndex(1);
                setup_axo_tab();
            } else if (text == str_2) {
                // edit axo
                axo_r_x = dx;
                axo_r_y = dy;
                ui->tabWidget->setTabEnabled(1, true);
                ui->tabWidget->setCurrentIndex(1);
                setup_axo_tab();
            } else if (text == str_3) {
            // make ortho
                orthos->set_Ortho(dx, -dy);
                if (dx == axo_r_x && dy == axo_r_y) {
                    axo_r_x = 0;
                    axo_r_y = 0;
                    ui->tabWidget->setTabEnabled(1, false);
                }
            }
        }
    }
}

void TaskOrthoViews::cb_toggled(bool toggle)
{
    QString name = sender()->objectName().right(2);
    char letter = name.toStdString()[0];
    int dx = letter - '0' - 2;

    letter = name.toStdString()[1];
    int dy = letter - '0' - 2;

    if (toggle) {
        orthos->add_view(dx, -dy);
        if (dx * dy != 0) {
             // adding an axo view
            axo_r_x = dx;
            axo_r_y = dy;
            ui->tabWidget->setTabEnabled(1, true);
            ui->tabWidget->setCurrentIndex(1);
            setup_axo_tab();
        }
    } else {
        // removing a view
        if (!orthos->is_Ortho(dx, -dy)) {
            // is it an axo one?
            if (dx == axo_r_x && dy == axo_r_y) {
                // is it the one currently being edited?
                axo_r_x = 0;
                axo_r_y = 0;
                ui->tabWidget->setTabEnabled(1, false);
            }
        }
        orthos->del_view(dx, -dy);
    }

    set_configs();
}

void TaskOrthoViews::projectionChanged(int index)
{
    int proj = 3 - 2 * index;       // index = 0 = third angle
    orthos->set_projection(proj);

    set_configs();
}

void TaskOrthoViews::setPrimary(int dir)
{
    int p_sel = ui->view_from->currentIndex();      // index for entry selected for 'view from'
    int r_sel = ui->axis_right->currentIndex();     // index for entry selected for 'rightwards axis'

    int p_vec[3] = {0, 0, 0};                       // will be the vector for 'view from'
    int r_vec[3] = {0, 0, 0};                       // will be vector for 'rightwards axis'
    int r[2] = {0, 1};

    int pos = 1 - 2 * int(p_sel / 3);               // 1 if p_sel = 0, 1, 2  or -1 if p_sel = 3, 4, 5
    p_sel = p_sel % 3;                              // p_sel = 0, 1, 2
    p_vec[p_sel] = pos;

    for (int i = p_sel; i < 2; i++)                 // make r[2] to be, {0, 1}, {0, 2}, or {1, 2} depending upon p_sel
        r[i] += 1;

    pos = 1 - 2 * int(r_sel / 2);                   // 1 if r_sel = 0, 1  or -1 if r_sel = 3, 4
    r_sel = r_sel % 2;                              // r_sel = 0, 1
    r_vec[r[r_sel]] = pos;

    gp_Dir facing = gp_Dir(p_vec[0], p_vec[1], p_vec[2]);
    gp_Dir right  = gp_Dir(r_vec[0], r_vec[1], r_vec[2]);

    orthos->set_primary(facing, right);

    // update rightwards combobox in case of 'view from' change
    if (sender() == ui->view_from)
    {
        disconnect(ui->axis_right, SIGNAL(currentIndexChanged(int)), this, SLOT(setPrimary(int)));

        QStringList items;
        items << QString::fromUtf8("X +ve") << QString::fromUtf8("Y +ve") << QString::fromUtf8("Z +ve");
        items << QString::fromUtf8("X -ve") << QString::fromUtf8("Y -ve") << QString::fromUtf8("Z -ve");
        items.removeAt(p_sel + 3);
        items.removeAt(p_sel);

        ui->axis_right->clear();
        ui->axis_right->addItems(items);
        ui->axis_right->setCurrentIndex(r_sel - pos + 1);

        connect(ui->axis_right, SIGNAL(currentIndexChanged(int)), this, SLOT(setPrimary(int)));
    }

    set_configs();
}

void TaskOrthoViews::hidden(int i)
{
    orthos->set_hidden(i == 2);
}


void TaskOrthoViews::smooth(int i)
{
    orthos->set_smooth(i == 2);
}

void TaskOrthoViews::toggle_auto(int i)
{
    if (i == 2) {
        //auto scale switched on
        orthos->auto_dims(true);
        ui->label_4->setEnabled(false);
        ui->label_5->setEnabled(false);
        ui->label_6->setEnabled(false);

        for (int j = 0; j < 5; j++)
            inputs[j]->setEnabled(false);       //disable user input boxes
    } else {
        orthos->auto_dims(false);
        ui->label_4->setEnabled(true);
        ui->label_5->setEnabled(true);
        ui->label_6->setEnabled(true);

        for (int j = 0; j < 5; j++)
            inputs[j]->setEnabled(true);        //enable user input boxes
    }
}

void TaskOrthoViews::data_entered(const QString &text)
{
    bool ok;
    QString name = sender()->objectName().right(1);
    char letter = name.toStdString()[0];
    int index = letter - '0';

    float value = text.toFloat(&ok);

    if (ok) {
        data[index] = value;
        orthos->set_configs(data);
    } else {
        inputs[index]->setText(QString::number(data[index]));
        return;
    }
}

void TaskOrthoViews::clean_up()
{
    orthos->del_all();
}

void TaskOrthoViews::setup_axo_tab()
{
    int     axo;
    gp_Dir  up, right;
    bool    away, tri;
    float   axo_scale;
    int     up_n, right_n;

    orthos->get_Axo(axo_r_x, -axo_r_y, axo, up, right, away, tri, axo_scale);

    // convert gp_Dirs into selections of comboboxes
    if (up.X() != 0)
        up_n = (up.X() == -1) ? 3 : 0;
    else if (up.Y() != 0)
        up_n = (up.Y() == -1) ? 4 : 1;
    else
        up_n = (up.Z() == -1) ? 5 : 2;

    if (right.X() != 0)
        right_n = (right.X() == -1) ? 3 : 0;
    else if (right.Y() != 0)
        right_n = (right.Y() == -1) ? 4 : 1;
    else
        right_n = (right.Z() == -1) ? 5 : 2;

    if (right_n > (up_n % 3 + 3))
        right_n -= 2;
    else if (right_n > up_n)
        right_n -= 1;

    QStringList items;
    items << QString::fromUtf8("X +ve") << QString::fromUtf8("Y +ve") << QString::fromUtf8("Z +ve");
    items << QString::fromUtf8("X -ve") << QString::fromUtf8("Y -ve") << QString::fromUtf8("Z -ve");
    items.removeAt(up_n % 3 + 3);
    items.removeAt(up_n % 3);

    ui->axoUp->setCurrentIndex(up_n);
    ui->axoRight->clear();
    ui->axoRight->addItems(items);
    ui->axoRight->setCurrentIndex(right_n);

    ui->vert_flip->setChecked(away);
    ui->tri_flip->setChecked(tri);
    ui->axoProj->setCurrentIndex(axo);
    ui->axoScale->setText(QString::number(axo_scale));
}

void TaskOrthoViews::change_axo(int p)
{
    int u_sel = ui->axoUp->currentIndex();        // index for entry selected for 'view from'
    int r_sel = ui->axoRight->currentIndex();         // index for entry selected for 'rightwards axis'

    int u_vec[3] = {0, 0, 0};               // will be the vector for 'view from'
    int r_vec[3] = {0, 0, 0};               // will be vector for 'rightwards axis'
    int r[2] = {0, 1};

    int pos = 1 - 2 * int(u_sel / 3);       // 1 if p_sel = 0,1,2  or -1 if p_sel = 3,4,5
    u_sel = u_sel % 3;                      // p_sel = 0,1,2
    u_vec[u_sel] = pos;

    for (int i = u_sel; i < 2; i++)
        r[i] += 1;

    pos = 1 - 2 * int(r_sel / 2);
    r_sel = r_sel % 2;
    r_vec[r[r_sel]] = pos;

    gp_Dir up = gp_Dir(u_vec[0], u_vec[1], u_vec[2]);
    gp_Dir right = gp_Dir(r_vec[0], r_vec[1], r_vec[2]);

    AxoMode axoType;
    switch(ui->axoProj->currentIndex())
    {
      case 0: axoType = ISOMETRIC;  break;
      case 1: axoType = DIMETRIC;   break;
      default: axoType = TRIMETRIC; break;
    }

    orthos->set_Axo(axo_r_x, -axo_r_y, up, right, ui->vert_flip->isChecked(), axoType, ui->tri_flip->isChecked());

    if (ui->axoProj->currentIndex() == 2)
        ui->tri_flip->setEnabled(true);
    else
        ui->tri_flip->setEnabled(false);

    QStringList items;
    items << QString::fromUtf8("X +ve") << QString::fromUtf8("Y +ve") << QString::fromUtf8("Z +ve");
    items << QString::fromUtf8("X -ve") << QString::fromUtf8("Y -ve") << QString::fromUtf8("Z -ve");
    items.removeAt(u_sel % 3 + 3);
    items.removeAt(u_sel % 3);

    ui->axoRight->clear();
    ui->axoRight->addItems(items);
    ui->axoRight->setCurrentIndex(r_sel - pos + 1);
}

void TaskOrthoViews::axo_button()
{
    change_axo();
}

void TaskOrthoViews::axo_scale(const QString & text)
{
    bool ok;
    float value = text.toFloat(&ok);

    if (ok)
        orthos->set_Axo_scale(axo_r_x, -axo_r_y, value);
}

void TaskOrthoViews::set_configs()
{
    orthos->get_configs(data);

    for(int i = 0; i < 5; i++)
        inputs[i]->setText(QString::number(data[i]));
}


bool TaskOrthoViews::user_input()
{
    if(txt_return) {
        txt_return = false;             // return was pressed while text box had focus
        ui->label_7->setFocus();        // move focus out of text box
        return true;                    // return that we were editing
    }
    else
        return false;                   // return that we weren't editing ---> treat as clicking OK... we can close the GUI
}

void TaskOrthoViews::text_return()
{
    txt_return = true;
}

#endif
//**************************************************************************


#include "moc_TaskOrthographicViews.cpp"
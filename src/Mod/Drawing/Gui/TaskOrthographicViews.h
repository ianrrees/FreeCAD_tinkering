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


class Ui_TaskOrthographicViews;

namespace App {
  class Document;
  class DocumentObject;
}

namespace Drawing {
  class FeatureViewPart;
}

namespace DrawingGui
{

enum AxoMode{
  ISOMETRIC = 0,
  DIMETRIC  = 1,
  TRIMETRIC = 2
};
#if 0

class Orthoview
{
public:
    Orthoview(App::DocumentObject *part, App::DocumentObject *page, Base::BoundBox3d *partbox);
    ~Orthoview();

public:
    void    remove();
    void    setData(const int &r_x, const int &r_y);
    void    setProjection(const gp_Ax2 &cs);
    void    setPos(const float &px, const float &py);
    void    setScale(const float &newScale);
    float   getScale() const;
    void    showHidden(bool state);
    void    showSmooth(bool state);

private:
    void    calcCentre(float &x, float &y) const;

public:     // these aren't used by orthoView, but just informational, hence public
    bool    ortho;          // orthonometric?  or axonometric
    bool    auto_scale;     // scale for axonometric has not been manually changed?
    int     rel_x, rel_y;   // relative position of this view
    bool    away, tri;      // binary parameters for axonometric view
    AxoMode axo;            // 0 / 1 / 2 = iso / di / tri metric
    gp_Dir  up, right;      // directions prior to rotations (ie, what was used to orientate the projection)

private:
    App::Document            *parent_doc;
    Drawing::FeatureViewPart *view;

    Base::Vector3f centerCoord;    // coords of bbox centre in 3D space

    float   scale;                  // scale of projection
    gp_Dir  X_dir, Y_dir, Z_dir;    // directions of projection, X_dir makes x on page, Y_dir is y on page, Z_dir is out of page
};

class OrthoViews
{
public:
    OrthoViews(const char * pagename, const char * partname);
    ~OrthoViews();

public:
    void    set_primary(gp_Dir facing, gp_Dir right);
    void    add_view(int rel_x, int rel_y);
    void    del_view(int rel_x, int rel_y);
    void    del_all();
    void    set_projection(int proj);
    void    set_hidden(bool state);
    void    set_smooth(bool state);
    void    set_Axo(int rel_x, int rel_y, gp_Dir up, gp_Dir right, bool away = false, AxoMode = ISOMETRIC, bool tri = false);
    void    set_Axo(int rel_x, int rel_y);
    void    set_Axo_scale(const int &rel_x, const int &rel_y, const float &axo_scale);
    void    set_Ortho(const int &rel_x, const int &rel_y);
    int     is_Ortho(int rel_x, int rel_y) const;
    bool    get_Axo(int rel_x, int rel_y, int & axo, gp_Dir & up, gp_Dir & right, bool & away, bool & tri, float & axo_scale);
    void    auto_dims(bool setting);
    void    set_configs(float configs[5]);
    void    get_configs(float configs[5]) const;

private:
    void    set_orientation(int index);
    void    load_page();                        // get page / titleblock dims from template
    void    choose_page();                      // determine correct portion of page to use to avoid interference with title block
    void    set_all_orientations();             // update orientations of all views following change in primary view
    void    calc_layout_size();                 // what's the real world size of chosen layout, excluding spaces
    void    calc_offsets();
    void    set_views();
    void    calc_scale();
    void    process_views();
    int     index(int rel_x, int rel_y) const;

    void getPageSize(double &width, double &height) const;
    void getMarginSize(double &marginX, double &marginY) const;

private:
    std::vector<Orthoview *>     views;
    Base::BoundBox3d             bbox;
    App::Document *              parent_doc;
    App::DocumentObject *        part;
    App::DocumentObject *        page;

    std::string page_name, part_name;

    int     large[4];                       // arrays containing page size info [margin_x, margin_y, size_x, size_y] = [x1, y1, x2-x1, y2-y1]
    int     small_h[4], small_v[4];         // page size avoiding title block, using maximum horizontal / vertical space
    int *   page_dims;                      // points to one of above arrays for which set of page dimensions to use
    int     block[4];                       // title block info [corner x, corner y, width, height], eg [-1, 1, w, h] is in top left corner
    bool    title;
    int *   horiz, * vert;                  // points to min or max r_x / r_y depending upon which corner title block is in

    int     rotate_coeff;                   // 1st (= -1) or 3rd (= 1) angle
    int     min_r_x, max_r_x;               // extreme relative positions of views
    int     min_r_y, max_r_y;               //      "       "       "
    float   width, height, depth;           // of non-scaled primary view
    float   layout_width, layout_height;    // of non-scaled layout without spaces
    float   gap_x, gap_y, min_space;        // required spacing between views
    float   offset_x, offset_y;             // coords of centre of upper left view
    float   scale;
    int     num_gaps_x, num_gaps_y;         // how many gaps between views/edges? = num of views in given direction + 1
    gp_Ax2  primary;                        // coord system of primary view

    bool    hidden, smooth;
    bool    autodims;
};

#endif

class TaskOrthographicViews : public QWidget//: public Gui::TaskView::TaskBox
{
    Q_OBJECT

public:
    TaskOrthographicViews(ViewProviderViewOrthographic  *orthographicView);
    ~TaskOrthographicViews();

public:
     void updateTask();
     void nearestFraction(const double &val, int &a, int &b, const long &maxDenom) const;

protected Q_SLOTS:
    void viewToggled(bool toggle);
    void scaleTypeChanged(int index);
    void scaleChanged(const QString & text);

#if 0
public:
    bool user_input();
    void clean_up();

protected Q_SLOTS:
    void ShowContextMenu(const QPoint & pos);
    void setPrimary(int dir);
    void cb_toggled(bool toggle);
    void projectionChanged(int index);
    void hidden(int i);
    void smooth(int i);
    void toggle_auto(int i);
    void data_entered(const QString & text);
    void change_axo(int p = 3);
    void axo_button();
    void axo_scale(const QString & text);
    void text_return();

#endif

protected:
    void changeEvent(QEvent * e);

#if 0
private:
    void setup_axo_tab();
    void set_configs();
#endif

private:
    //class Private;
    Ui_TaskOrthographicViews * ui;
    bool blockUpdate;

protected:
  ViewProviderViewOrthographic *orthographicView;
};

/// Simulation dialog for the TaskView
class TaskDlgOrthographicViews : public Gui::TaskView::TaskDialog
{
    Q_OBJECT

public:
    TaskDlgOrthographicViews(ViewProviderViewOrthographic *vp);
    ~TaskDlgOrthographicViews();

    ViewProviderViewOrthographic * getOrthographicView() const { return orthographicView; }

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
    ViewProviderViewOrthographic *orthographicView;
private:
    TaskOrthographicViews * widget;
    Gui::TaskView::TaskBox* taskbox;
};

} //namespace DrawingGui

#endif // GUI_TASKVIEW_OrthographicViews_H

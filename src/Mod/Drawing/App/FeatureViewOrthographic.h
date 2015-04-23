/***************************************************************************
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

#ifndef _DRAWING_FEATUREVIEWORTHOGRAPHIC_h_
#define _DRAWING_FEATUREVIEWORTHOGRAPHIC_h_

#include <App/DocumentObject.h>
#include <App/PropertyStandard.h>
#include <App/FeaturePython.h>

#include <Base/BoundBox.h>
#include <Base/Matrix.h>
#include "FeatureViewCollection.h"
#include "FeatureOrthoView.h"

namespace Drawing
{

/**
 * Class super-container for managing a collection of FeatureOrthoView
 * Page Features
 */
class DrawingExport FeatureViewOrthographic : public Drawing::FeatureViewCollection
{
    PROPERTY_HEADER(Drawing::FeatureViewOrthographic);

public:
    /// Constructor
    FeatureViewOrthographic();
    ~FeatureViewOrthographic();

    App::PropertyEnumeration ProjectionType;

    /// Transforms Direction and XAxisDirection vectors in child views
    App::PropertyMatrix viewOrientationMatrix;

    App::PropertyLink Anchor; /// Anchor Element to align views to

    Base::BoundBox3d getBoundingBox() const;
    double calculateAutomaticScale() const;

    // Check if container has a view of a specific type
    bool hasOrthoView(const char *viewProjType) const;
    App::DocumentObject * getOrthoView(const char *viewProjType) const;

    App::DocumentObject * addOrthoView(const char *viewProjType);

    //! Removes an ortho view
    /*!
     * \return number of ortho views remaining
     */
    int  removeOrthoView(const char *viewProjType);

    /// Automatically position child views
    bool distributeOrthoViews(void);

    /// Changes child views' coordinate space
    /*!
     * Used to set the Direction and XAxisDirection in child views
     */
    void setFrontViewOrientation(const Base::Matrix4D &newMat);

    short mustExecute() const;
    /** @name methods overide Feature */
    //@{
    /// recalculate the Feature
    virtual void onDocumentRestored();
    virtual App::DocumentObjectExecReturn *execute(void);
    //@}

    /// returns the type name of the ViewProvider
    virtual const char* getViewProviderName(void) const {
        return "DrawingGui::ViewProviderViewOrthographic";
    }

protected:
    void onChanged(const App::Property* prop);

    //! Moves anchor view to keep our bounding box centre on the origin
    void moveToCentre();

    /// Annoying helper - keep in sync with FeatureOrthoView::TypeEnums
    /*!
     * \TODO See note regarding App::PropertyEnumeration on my wiki page http://freecadweb.org/wiki/index.php?title=User:Ian.rees
     * \return true iff 'in' is a valid name for an orthographic/isometric view
     */
    bool checkViewProjType(const char *in);

    /// Sets Direction and XAxisDirection in v
    /*!
     * Applies viewOrientationMatrix to appropriate unit vectors depending on projType
     */
    void setViewOrientation(FeatureOrthoView *v, const char *projType) const;

    /// Populates an array of FeatureOrthoView*s arranged for drawing
    /*!
     * Setup array of pointers to the views that we're displaying,
     * assuming front is in centre (index 4):
     * <pre>
     * [0]  [1]  [2]
     * [3]  [4]  [5]  [6]
     * [7]  [8]  [9]
     *
     * Third Angle:  FTL  T  FTRight
     *                L   F   Right   Rear
     *               FBL  B  FBRight
     *
     * First Angle:  FBRight  B  FBL
     *                Right   F   L  Rear
     *               FTRight  T  FTL
     * </pre>
     */
    void arrangeViewPointers(FeatureOrthoView *viewPtrs[10]) const;

    /// Populates array of 10 BoundBox3d's given FeatureOrthoView *s
    /*!
     * If documentScale is set, then returned bounding boxes are scaled as in
     * the Drawing.  Otherwise, the dimensions are as in object space.
     */
    void makeViewBbs(FeatureOrthoView *viewPtrs[10],
                     Base::BoundBox3d bboxes[10],
                     bool documentScale = true) const;

    /// Helper for calculateAutomaticScale
    /*!
     * Returns a width and height in object-space scale, for the enabled views
     * without accounting for their actual X and Y positions or borders.
     */
    void minimumBbViews(FeatureOrthoView *viewPtrs[10],
                       double &width, double &height) const;
private:
    static const char* ProjectionTypeEnums[];
    static const char* OrthoViewNameEnumStrs[];

};

} //namespace Drawing

#endif // _DRAWING_FEATUREVIEWORTHOGRAPHIC_h_

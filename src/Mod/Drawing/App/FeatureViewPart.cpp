/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2002     *
 *   Copyright (c) Luke Parry             (l.parry@warwick.ac.uk) 2013     *
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
# include <sstream>
#endif


#include <HLRBRep_Algo.hxx>
#include <TopoDS_Shape.hxx>
#include <HLRTopoBRep_OutLiner.hxx>
//#include <BRepAPI_MakeOutLine.hxx>
#include <HLRAlgo_Projector.hxx>
#include <HLRBRep_ShapeBounds.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <BRep_Tool.hxx>

#include <Base/BoundBox.h>
#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/FileInfo.h>
#include <Mod/Part/App/PartFeature.h>

#include "Geometry.h"
#include "FeatureViewPart.h"
#include "ProjectionAlgos.h"
#include "FeatureHatch.h"
//#include "FeatureViewDimension.h"

using namespace Drawing;
using namespace std;


//===========================================================================
// FeatureViewPart
//===========================================================================

Base::Vector3d _getValidXDir(const FeatureViewPart *me);

App::PropertyFloatConstraint::Constraints FeatureViewPart::floatRange = {0.01f,5.0f,0.05f};

PROPERTY_SOURCE(Drawing::FeatureViewPart, Drawing::FeatureView)

FeatureViewPart::FeatureViewPart(void) : geometryObject(0)
{
    static const char *group = "Shape view";
    static const char *vgroup = "Drawing view";

    ADD_PROPERTY_TYPE(Direction ,(0,0,1.0)    ,group,App::Prop_None,"Projection normal direction");
    ADD_PROPERTY_TYPE(Source ,(0),group,App::Prop_None,"3D Shape to view");
    ADD_PROPERTY_TYPE(ShowHiddenLines ,(false),group,App::Prop_None,"Control the appearance of the dashed hidden lines");
    ADD_PROPERTY_TYPE(ShowSmoothLines ,(false),group,App::Prop_None,"Control the appearance of the smooth lines");
    ADD_PROPERTY_TYPE(LineWidth,(0.7f),vgroup,App::Prop_None,"The thickness of the resulting lines");
    ADD_PROPERTY_TYPE(HiddenWidth,(0.15),vgroup,App::Prop_None,"The thickness of the hidden lines, if enabled");
    ADD_PROPERTY_TYPE(Tolerance,(0.05f),vgroup,App::Prop_None,"The tessellation tolerance");
    Tolerance.setConstraints(&floatRange);
    ADD_PROPERTY_TYPE(XAxisDirection ,(1,0,0) ,group,App::Prop_None,"X-Axis direction");
    //ADD_PROPERTY_TYPE(HatchAreas ,(0),vgroup,App::Prop_None,"Hatched areas of this view");

    geometryObject = new DrawingGeometry::GeometryObject();
}

FeatureViewPart::~FeatureViewPart()
{
    delete geometryObject;
}


App::DocumentObjectExecReturn *FeatureViewPart::execute(void)
{
    App::DocumentObject *link = Source.getValue();
    if (!link) {
        return new App::DocumentObjectExecReturn("FVP - No Source object linked");
    }

    if (!link->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId())) {
        return new App::DocumentObjectExecReturn("FVP - Linked object is not a Part object");
    }

    TopoDS_Shape shape = static_cast<Part::Feature*>(link)->Shape.getShape()._Shape;
    if (shape.IsNull()) {
        return new App::DocumentObjectExecReturn("FVP - Linked shape object is empty");
    }

    try {
        geometryObject->setTolerance(Tolerance.getValue());
        geometryObject->setScale(Scale.getValue());
        geometryObject->extractGeometry(shape,
                                        Direction.getValue(),
                                        ShowHiddenLines.getValue(),
                                        _getValidXDir(this));
        bbox = geometryObject->calcBoundingBox();
        touch();

    }
    catch (Standard_Failure) {
        Handle_Standard_Failure e = Standard_Failure::Caught();
        return new App::DocumentObjectExecReturn(e->GetMessageString());
    }


    // There is a guaranteed change so check any references linked to this and touch
    // We need to update all views pointing at this (ProjectionGroup, ClipGroup, etc)
    std::vector<App::DocumentObject*> parent = getInList();
    for (std::vector<App::DocumentObject*>::iterator it = parent.begin(); it != parent.end(); ++it) {
        if ((*it)->getTypeId().isDerivedFrom(FeatureView::getClassTypeId())) {
            Drawing::FeatureView *view = static_cast<Drawing::FeatureView *>(*it);
            view->touch();
        }
    }

    return FeatureView::execute();
}

short FeatureViewPart::mustExecute() const
{
    short result  = (Direction.isTouched() ||
            XAxisDirection.isTouched() ||
            Source.isTouched() ||
            Scale.isTouched() ||
            ScaleType.isTouched() ||
            ShowHiddenLines.isTouched());
    return result;
}

void FeatureViewPart::onChanged(const App::Property* prop)
{
    if (!isRestoring()) {
        if (prop == &Direction ||
            prop == &XAxisDirection ||
            prop == &Source ||
            prop == &Scale ||
            prop == &ScaleType ||
            prop == &ShowHiddenLines) {
            try {
                App::DocumentObjectExecReturn *ret = recompute();
                delete ret;
            }
            catch (...) {
            }
        }
    }
    FeatureView::onChanged(prop);

//TODO: when scale changes, any Dimensions for this View sb recalculated.
}

#if 0
int FeatureViewPart::addHatch(App::DocumentObject *docObj)
{
    if(!docObj->isDerivedFrom(Drawing::FeatureHatch::getClassTypeId()))
        return -1;

    const std::vector<App::DocumentObject *> currAreas = HatchAreas.getValues();
    std::vector<App::DocumentObject *> newAreas(currAreas);
    newAreas.push_back(docObj);
    HatchAreas.setValues(newAreas);
    HatchAreas.touch();
    return HatchAreas.getSize();
}

int FeatureViewPart::removeHatch(App::DocumentObject *docObj)
{
    if(!docObj->isDerivedFrom(Drawing::FeatureHatch::getClassTypeId()))
        return -1;

    const std::vector<App::DocumentObject*> currAreas = HatchAreas.getValues();
    std::vector<App::DocumentObject*> newAreas;
    std::vector<App::DocumentObject*>::const_iterator it = currAreas.begin();
    for (; it != currAreas.end(); it++) {
        std::string areaName = docObj->getNameInDocument();
        if (areaName.compare((*it)->getNameInDocument()) != 0) {
            newAreas.push_back((*it));
        }
    }
    HatchAreas.setValues(newAreas);
    HatchAreas.touch();

    return HatchAreas.getSize();
}
#endif

std::vector<Drawing::FeatureHatch*> FeatureViewPart::getHatches() const
{
    std::vector<Drawing::FeatureHatch*> result;
    std::vector<App::DocumentObject*> children = getInList();
    for (std::vector<App::DocumentObject*>::iterator it = children.begin(); it != children.end(); ++it) {
        if ((*it)->getTypeId().isDerivedFrom(FeatureHatch::getClassTypeId())) {
            Drawing::FeatureHatch* hatch = dynamic_cast<Drawing::FeatureHatch*>(*it);
            result.push_back(hatch);
        }
    }

    return result;
}

const std::vector<DrawingGeometry::Vertex *> & FeatureViewPart::getVertexGeometry() const
{
    return geometryObject->getVertexGeometry();
}

const std::vector<int> & FeatureViewPart::getVertexReferences() const
{
    return geometryObject->getVertexRefs();
}

const std::vector<DrawingGeometry::Face *> & FeatureViewPart::getFaceGeometry() const
{
    return geometryObject->getFaceGeometry();
}

const std::vector<int> & FeatureViewPart::getFaceReferences() const
{
    return geometryObject->getFaceRefs();
}

const std::vector<DrawingGeometry::BaseGeom  *> & FeatureViewPart::getEdgeGeometry() const
{
    return geometryObject->getEdgeGeometry();
}

const std::vector<int> & FeatureViewPart::getEdgeReferences() const
{
    return geometryObject->getEdgeRefs();
}

//! project Source Edge(idx) to 2D BaseGeom
DrawingGeometry::BaseGeom *FeatureViewPart::getCompleteEdge(int idx) const
{
   //NOTE: idx is in fact a Reference to an Edge in Source
   //returns projection of ref'd Edge as BaseGeom. Why not just use existing BaseGeom(idx)?
    App::DocumentObject* link = Source.getValue();

    if (!link || !link->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
        return 0;

    const Part::TopoShape &topoShape = static_cast<Part::Feature*>(link)->Shape.getShape();
    std::stringstream str;
    str << "Edge" << idx;
    TopoDS_Shape shape = topoShape.getSubShape(str.str().c_str());


    const TopoDS_Shape &support = static_cast<Part::Feature*>(link)->Shape.getValue();
    //TODO: make sure prjShape gets deleted

    DrawingGeometry::BaseGeom* prjShape = 0;
    try {
        prjShape = geometryObject->projectEdge(shape, support, Direction.getValue(), _getValidXDir(this));
    }
    catch(Standard_Failure) {
        Base::Console().Error("getCompleteEdge - OCC Error - could not project Edge: %d\n",idx);
        return 0;
    }
    catch (exception& e) {
        Base::Console().Error("getCompleteEdge - unknown exception on Edge: %d - %s\n",idx,e.what());
        return 0;
    }
    catch(...) {
        Base::Console().Error("getCompleteEdge - unknown error on Edge: %d\n",idx);
        return 0;
    }

    return prjShape;
}

//! project Source Vertex(idx) to 2D geometry
DrawingGeometry::Vertex * FeatureViewPart::getVertex(int idx) const
{
   //## Get the Part Link ##/
    App::DocumentObject* link = Source.getValue();

    if (!link || !link->getTypeId().isDerivedFrom(Part::Feature::getClassTypeId()))
        return 0;

    const Part::TopoShape &topoShape = static_cast<Part::Feature*>(link)->Shape.getShape();
    std::stringstream str;
    str << "Vertex" << idx;
    TopoDS_Shape shape = topoShape.getSubShape(str.str().c_str());

    const TopoDS_Shape &support = static_cast<Part::Feature*>(link)->Shape.getValue();
    //TODO: Make sure prjShape gets deleted
    DrawingGeometry::Vertex *prjShape = geometryObject->projectVertex(shape, support, Direction.getValue(), _getValidXDir(this));
    //Base::Console().Log("vert %f, %f \n", prjShape->pnt.fX,  prjShape->pnt.fY);
    return prjShape;
}

DrawingGeometry::Vertex* FeatureViewPart::getVertexGeomByRef(int ref) const
{
    const std::vector<DrawingGeometry::Vertex *> &verts = getVertexGeometry();
    if (verts.empty()) {
        Base::Console().Log("INFO - getVertexGeomByRef(%d) - no Vertex Geometry. Probably restoring?\n",ref);
        return NULL;
    }
    const std::vector<int> &vertRefs                    = getVertexReferences();
    std::vector<DrawingGeometry::Vertex *>::const_iterator vert = verts.begin();
    bool found = false;
    for(int i = 0 ; vert != verts.end(); ++vert, i++) {
        if (vertRefs[i] == ref) {
            found = true;
            break;
        }
    }
    if (found) {
        return (*vert);
    } else {
        std::stringstream error;
        error << "getVertexGeomByRef: no vertex geometry for ref: " << ref;
        throw Base::Exception(error.str().c_str());
    }
}

//! returns existing BaseGeom of Edge with 3D Reference = ref
DrawingGeometry::BaseGeom* FeatureViewPart::getEdgeGeomByRef(int ref) const
{
    const std::vector<DrawingGeometry::BaseGeom *> &geoms = getEdgeGeometry();
    if (geoms.empty()) {
        Base::Console().Log("INFO - getEdgeGeomByRef(%d) - no Edge Geometry. Probably restoring?\n",ref);
        return NULL;
    }
    const std::vector<int> &refs = getEdgeReferences();
    std::vector<DrawingGeometry::BaseGeom*>::const_iterator it = geoms.begin();
    bool found = false;
    for(int i = 0 ; it != geoms.end(); ++it, i++) {
        if (refs[i] == ref) {
            found = true;
            break;
        }
    }
    if (found) {
        return (*it);
    } else {
        std::stringstream error;
        error << "getEdgeGeomByRef: no edge geometry for ref: " << ref;
        throw Base::Exception(error.str().c_str());
    }
}

//! returns existing BaseGeom of 2D Edge(idx)
DrawingGeometry::BaseGeom* FeatureViewPart::getProjEdgeByIndex(int idx) const
{
    const std::vector<DrawingGeometry::BaseGeom *> &geoms = getEdgeGeometry();
    if (geoms.empty()) {
        Base::Console().Log("INFO - getProjEdgeByIndex(%d) - no Edge Geometry. Probably restoring?\n",idx);
        return NULL;
    }
    return geoms[idx];
}

//! returns existing geometry of 2D Vertex(idx)
DrawingGeometry::Vertex* FeatureViewPart::getProjVertexByIndex(int idx) const
{
    const std::vector<DrawingGeometry::Vertex *> &geoms = getVertexGeometry();
    if (geoms.empty()) {
        Base::Console().Log("INFO - getProjVertexByIndex(%d) - no Vertex Geometry. Probably restoring?\n",idx);
        return NULL;
    }
    return geoms[idx];
}

int FeatureViewPart::getEdgeRefByIndex(int idx) const
{
    const std::vector<int> &refs = getEdgeReferences();
    if (refs.empty()) {
        Base::Console().Log("INFO - getEdgeRefByIndex(%d) - no Edge Geometry. Probably restoring?\n",idx);
        return -1;
    }
    return refs[idx];
}

int FeatureViewPart::getVertexRefByIndex(int idx) const
{
    const std::vector<int> &refs = getVertexReferences();
    if (refs.empty()) {
        Base::Console().Log("INFO - getVertexRefByIndex(%d) - no Vertex Geometry. Probably restoring?\n",idx);
        return -1;
    }
    return refs[idx];
}

Base::BoundBox3d FeatureViewPart::getBoundingBox() const
{
    return bbox;
}

bool FeatureViewPart::hasGeometry(void) const
{
    bool result = false;
    const std::vector<DrawingGeometry::Vertex*> &verts = getVertexGeometry();
    const std::vector<DrawingGeometry::BaseGeom*> &edges = getEdgeGeometry();
    if (verts.empty() &&
        edges.empty() ) {
        result = false;
    } else {
        result = true;
    }
    return result;
}

Base::Vector3d _getValidXDir(const FeatureViewPart *me)
{
    Base::Vector3d xDir = me->XAxisDirection.getValue();
    if (xDir.Length() == 0) {
        Base::Console().Warning("XAxisDirection has zero length - using (1,0,0)\n");
        xDir = Base::Vector3d(1.0,0.0,0.0);
    }
    return xDir;
}

void FeatureViewPart::dumpVertexRefs(char* text) const
{
    Base::Console().Message("DUMP - %s\n",text);
    const std::vector<int> &refs = getVertexReferences();
    std::vector<int>::const_iterator it = refs.begin();
    int i = 0;
    for( ; it != refs.end(); it++, i++) {
        Base::Console().Message("DUMP - Vertex: %d ref: %d\n",i,(*it));
    }
}

// Python Drawing feature ---------------------------------------------------------

namespace App {
/// @cond DOXERR
PROPERTY_SOURCE_TEMPLATE(Drawing::FeatureViewPartPython, Drawing::FeatureViewPart)
template<> const char* Drawing::FeatureViewPartPython::getViewProviderName(void) const {
    return "DrawingGui::ViewProviderView";
}
/// @endcond

// explicit template instantiation
template class DrawingExport FeaturePythonT<Drawing::FeatureViewPart>;
}

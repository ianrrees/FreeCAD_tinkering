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

# include "PreCompiled.h"
#ifndef _PreComp_

# include <gp_Ax2.hxx>
# include <gp_Circ.hxx>
# include <gp_Dir.hxx>
# include <gp_Elips.hxx>
# include <gp_Pln.hxx>
# include <gp_Pnt.hxx>
# include <gp_Vec.hxx>

# include <Bnd_Box.hxx>
# include <BRepBndLib.hxx>
# include <BRepBuilderAPI_Transform.hxx>

# include <HLRTopoBRep_OutLiner.hxx>
# include <HLRBRep.hxx>
# include <HLRBRep_Algo.hxx>
# include <HLRBRep_Data.hxx>
# include <HLRBRep_EdgeData.hxx>
# include <HLRAlgo_EdgeIterator.hxx>
# include <HLRBRep_HLRToShape.hxx>
# include <HLRAlgo_Projector.hxx>
# include <HLRBRep_ShapeBounds.hxx>

# include <Poly_Polygon3D.hxx>
# include <Poly_Triangulation.hxx>
# include <Poly_PolygonOnTriangulation.hxx>

# include <TopoDS.hxx>
# include <TopoDS_Face.hxx>
# include <TopoDS_Edge.hxx>
# include <TopoDS_Vertex.hxx>
# include <TopExp.hxx>
# include <TopExp_Explorer.hxx>
# include <TopoDS_Shape.hxx>
# include <TopTools_ListIteratorOfListOfShape.hxx>
# include <TopTools_IndexedMapOfShape.hxx>
# include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
# include <TopTools_ListOfShape.hxx>
# include <TColgp_Array1OfPnt2d.hxx>

# include <BRep_Tool.hxx>
# include <BRepMesh.hxx>
# include <BRepMesh_IncrementalMesh.hxx>
# include <BRep_Builder.hxx>
# include <BRepBuilderAPI_MakeWire.hxx>
# include <BRepTools_WireExplorer.hxx>
# include <ShapeFix_Wire.hxx>
# include <BRepProj_Projection.hxx>

# include <BRepAdaptor_HCurve.hxx>
# include <BRepAdaptor_CompCurve.hxx>

// # include <Handle_BRepAdaptor_HCompCurve.hxx>
# include <Approx_Curve3d.hxx>

# include <BRepAdaptor_HCurve.hxx>
# include <Handle_HLRBRep_Data.hxx>
# include <Geom_BSplineCurve.hxx>
# include <Geom_BezierCurve.hxx>
# include <GeomConvert_BSplineCurveToBezierCurve.hxx>
# include <GeomConvert_BSplineCurveKnotSplitting.hxx>
# include <Geom2d_BSplineCurve.hxx>

#include <ProjLib_Plane.hxx>
#endif

# include <Base/Console.h>
# include <Base/Exception.h>
# include <Base/FileInfo.h>
# include <Base/Tools.h>

# include <Mod/Part/App/PartFeature.h>

# include "GeometryObject.h"

using namespace DrawingGeometry;

struct EdgePoints {
    gp_Pnt v1, v2;
    TopoDS_Edge edge;
};

GeometryObject::GeometryObject() : brep_hlr(0), Tolerance(0.05f), Scale(1.f)
{
}

GeometryObject::~GeometryObject()
{
    clear();
}

void GeometryObject::setTolerance(double value)
{
    Tolerance = value;
}

void GeometryObject::setScale(double value)
{
    Scale = value;
}

void GeometryObject::clear()
{

    for(std::vector<BaseGeom *>::iterator it = edgeGeom.begin(); it != edgeGeom.end(); ++it) {
        delete *it;
        *it = 0;
    }

    for(std::vector<Face *>::iterator it = faceGeom.begin(); it != faceGeom.end(); ++it) {
        delete *it;
        *it = 0;
    }

    for(std::vector<Vertex *>::iterator it = vertexGeom.begin(); it != vertexGeom.end(); ++it) {
        delete *it;
        *it = 0;
    }

    vertexGeom.clear();
    vertexReferences.clear();

    faceGeom.clear();
    faceReferences.clear();

    edgeGeom.clear();
    edgeReferences.clear();
}

TopoDS_Shape GeometryObject::invertY(const TopoDS_Shape& shape) const
{
    // make sure to have the y coordinates inverted
    gp_Trsf mat;
    Bnd_Box bounds;
    BRepBndLib::Add(shape, bounds);
    bounds.SetGap(0.0);
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    mat.SetMirror(gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2), gp_Dir(0,1,0)));
    BRepBuilderAPI_Transform mkTrf(shape, mat);
    return mkTrf.Shape();
}

void GeometryObject::drawFace (const bool visible, const int typ, const int iface, Handle_HLRBRep_Data & DS, TopoDS_Shape& Result) const
{
// add all the edges for this face(iface) to Result
    HLRBRep_FaceIterator Itf;

    for (Itf.InitEdge(DS->FDataArray().ChangeValue(iface)); Itf.MoreEdge(); Itf.NextEdge()) {
        int ie = Itf.Edge();
        HLRBRep_EdgeData& edf = DS->EDataArray().ChangeValue(ie);
        drawEdge(edf, Result, visible);
    }
}

void GeometryObject::drawEdge(HLRBRep_EdgeData& ed, TopoDS_Shape& Result, const bool visible) const
{
// add (visible) intervals of ed to Result as Edges
    double sta,end;
    float tolsta,tolend;

    BRep_Builder B;
    TopoDS_Edge E;
    HLRAlgo_EdgeIterator It;

    if (visible) {
        for(It.InitVisible(ed.Status()); It.MoreVisible(); It.NextVisible()) {
            It.Visible(sta,tolsta,end,tolend);
            E = HLRBRep::MakeEdge(ed.Geometry(),sta,end);
            if (!E.IsNull()) {
                B.Add(Result,E);
            }
        }
    } else {
        for(It.InitHidden(ed.Status()); It.MoreHidden(); It.NextHidden()) {
            It.Hidden(sta,tolsta,end,tolend);
            E = HLRBRep::MakeEdge(ed.Geometry(),sta,end);
            if (!E.IsNull()) {
                B.Add(Result,E);
            }
        }
    }
}

DrawingGeometry::Vertex * GeometryObject::projectVertex(const TopoDS_Shape &vert, const TopoDS_Shape &support, const Base::Vector3d &direction) const
{
    if(vert.IsNull())
        throw Base::Exception("Projected vertex is null");
    // Inverty y function using support to calculate bounding box
    gp_Trsf mat;
    Bnd_Box bounds;
    BRepBndLib::Add(support, bounds);
    bounds.SetGap(0.0);
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    mat.SetMirror(gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2), gp_Dir(0,1,0)));

    gp_Trsf matScale;
    matScale.SetScaleFactor(Scale);

    BRepBuilderAPI_Transform mkTrf(vert, mat);
    BRepBuilderAPI_Transform mkTrfScale(mkTrf.Shape(), matScale);

    const TopoDS_Vertex &refVert = TopoDS::Vertex(mkTrfScale.Shape());

    gp_Ax2 transform;
    if(projXAxis.Length() > FLT_EPSILON) {
        transform = gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2),
                           gp_Dir(direction.x, direction.y, direction.z),
                           gp_Dir(projXAxis.x, projXAxis.y, projXAxis.z));
    } else {
        transform = gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2),
                           gp_Dir(direction.x, direction.y, direction.z));
    }

    HLRAlgo_Projector projector = HLRAlgo_Projector( transform );
    projector.Scaled(true);
    // If the index was found and is unique, the point is projected using the HLR Projector Algorithm
    gp_Pnt2d prjPnt;
    projector.Project(BRep_Tool::Pnt(refVert), prjPnt);
    DrawingGeometry::Vertex *myVert = new Vertex(prjPnt.X(), prjPnt.Y());
    return myVert;
}

void GeometryObject::projectSurfaces(const TopoDS_Shape &face,
                                     const TopoDS_Shape &support,
                                     const Base::Vector3d &direction,
                                     const Base::Vector3d &xaxis,
                                     std::vector<DrawingGeometry::Face *> &projFaces) const
{
    if(face.IsNull())
        throw Base::Exception("Projected shape is null");
    // Inverty y function using support to calculate bounding box

    gp_Trsf mat;
    Bnd_Box bounds;
    BRepBndLib::Add(invertY(support), bounds);
    bounds.SetGap(0.0);
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    mat.SetMirror(gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2), gp_Dir(0,1,0)));
    gp_Trsf matScale;
    matScale.SetScaleFactor(Scale);

    BRepBuilderAPI_Transform mkTrf(face, mat);
    BRepBuilderAPI_Transform mkTrfScale(mkTrf.Shape(), matScale);

    gp_Ax2 transform;
    if(xaxis.Length() > FLT_EPSILON) {
        transform = gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2),
                           gp_Dir(direction.x, direction.y, direction.z),
                           gp_Dir(xaxis.x, xaxis.y, xaxis.z));
    } else {
        transform = gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2),
                           gp_Dir(direction.x, direction.y, direction.z));
    }

    HLRBRep_Algo *brep_hlr = new HLRBRep_Algo();
    brep_hlr->Add(mkTrfScale.Shape());

    HLRAlgo_Projector projector( transform );
    brep_hlr->Projector(projector);
    brep_hlr->Update();
    brep_hlr->Hide();

    Base::Console().Log("projecting face");

    // Extract Faces
    std::vector<int> projFaceRefs;

    extractFaces(brep_hlr, mkTrfScale.Shape(), 5, true, WithSmooth, projFaces, projFaceRefs);  //
    delete brep_hlr;
}

Base::BoundBox3d GeometryObject::calcBoundingBox() const
{
    Base::BoundBox3d bbox(0., 0., 0.);

    const std::vector<Vertex *> verts = vertexGeom;
    const std::vector<BaseGeom *> edges = edgeGeom;

    // First calculate bounding box based on vertices
    for(std::vector<Vertex *>::const_iterator it = verts.begin(); it != verts.end(); ++it) {
        bbox.Add(Base::Vector3d((*it)->pnt.fX, (*it)->pnt.fY, 0.));
    }

    // Now, consider geometry where vertices don't define bounding box eg circles
    for(std::vector<BaseGeom *>::const_iterator it = edges.begin(); it != edges.end(); ++it) {
        Base::BoundBox3d bb;
        switch ((*it)->geomType) {
          case CIRCLE: {
              Circle *circ = static_cast<Circle *>(*it);
              bb = Base::BoundBox3d(0., 0., 0, circ->radius*2., circ->radius*2, 0.);
              bb.MoveX(circ->center.fX);
              bb.MoveY(circ->center.fY);

          } break;
          case ELLIPSE: {
              Ellipse *circ = static_cast<Ellipse *>(*it);
                // TODO: double check this - seems like it should have a MoveX, MoveY
              bb = Base::BoundBox3d(0., 0., 0, circ->minor*2., circ->major*2, 0.);
          } break;
          default:
              break;
        }

        bbox.Add(bb);
    }
    return bbox;
}


DrawingGeometry::BaseGeom * GeometryObject::projectEdge(const TopoDS_Shape &edge,
                                                        const TopoDS_Shape &support,
                                                        const Base::Vector3d &direction) const
{
    if(edge.IsNull())
        throw Base::Exception("Projected edge is null");
    // Invert y function using support to calculate bounding box
    // TODO: look at using invertY()

    gp_Trsf mat;
    Bnd_Box bounds;
    BRepBndLib::Add(support, bounds);
    bounds.SetGap(0.0);
    Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
    bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    mat.SetMirror(gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2), gp_Dir(0.f,1.f,0.f)));

    gp_Trsf matScale;
    matScale.SetScaleFactor(Scale);

    BRepBuilderAPI_Transform mkTrf(edge, mat);
    BRepBuilderAPI_Transform mkTrfScale(mkTrf.Shape(), matScale);

    const TopoDS_Edge &refEdge = TopoDS::Edge(mkTrfScale.Shape());

    gp_Ax2 transform;
    if(projXAxis.Length() > FLT_EPSILON) {
        transform = gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2),
                           gp_Dir(direction.x, direction.y, direction.z),
                           gp_Dir(projXAxis.x, projXAxis.y, projXAxis.z));
    } else {
        transform = gp_Ax2(gp_Pnt((xMin+xMax)/2,(yMin+yMax)/2,(zMin+zMax)/2),
                           gp_Dir(direction.x, direction.y, direction.z));
    }

    BRepAdaptor_Curve refCurve(refEdge);
    HLRAlgo_Projector projector = HLRAlgo_Projector( transform );
    projector.Scaled(true);

    if (refCurve.GetType() == GeomAbs_Line) {

        // Use the simpler algorithm for lines
        gp_Pnt p1 = refCurve.Value(refCurve.FirstParameter());
        gp_Pnt p2 = refCurve.Value(refCurve.LastParameter());

        // Project the points
        gp_Pnt2d pnt1, pnt2;
        projector.Project(p1, pnt1);
        projector.Project(p2, pnt2);

        DrawingGeometry::Generic *line = new DrawingGeometry::Generic();

        line->points.push_back(Base::Vector2D(pnt1.X(), pnt1.Y()));
        line->points.push_back(Base::Vector2D(pnt2.X(), pnt2.Y()));

        return line;

    } else {

        HLRBRep_Curve curve;
        curve.Curve(refEdge);

        curve.Projector(&projector);

        DrawingGeometry::BaseGeom *result = 0;
        switch(HLRBRep_BCurveTool::GetType(curve.Curve()))
        {
            case GeomAbs_Line: {
              DrawingGeometry::Generic *line = new DrawingGeometry::Generic();

              gp_Pnt2d pnt1 = curve.Value(curve.FirstParameter());
              gp_Pnt2d pnt2 = curve.Value(curve.LastParameter());

              line->points.push_back(Base::Vector2D(pnt1.X(), pnt1.Y()));
              line->points.push_back(Base::Vector2D(pnt2.X(), pnt2.Y()));

              result = line;
            }break;
        case GeomAbs_Circle: {
              DrawingGeometry::Circle *circle = new DrawingGeometry::Circle();
                gp_Circ2d prjCirc = curve.Circle();

                double f = curve.FirstParameter();
                double l = curve.LastParameter();
                gp_Pnt2d s = curve.Value(f);
                gp_Pnt2d e = curve.Value(l);

                if (fabs(l-f) > 1.0 && s.SquareDistance(e) < 0.001) {
                      Circle *geom = new Circle();
                      circle->radius = prjCirc.Radius();
                      circle->center = Base::Vector2D(prjCirc.Location().X(), prjCirc.Location().Y());
                      result = circle;
                } else {
                      AOC *aoc = new AOC();
                      aoc->radius = prjCirc.Radius();
                      aoc->center = Base::Vector2D(prjCirc.Location().X(), prjCirc.Location().Y());
                      double ax = s.X() - aoc->center.fX;
                      double ay = s.Y() - aoc->center.fY;
                      double bx = e.X() - aoc->center.fX;
                      double by = e.Y() - aoc->center.fY;

                      aoc->startAngle = atan2(ay,ax);
                      float range = atan2(-ay*bx+ax*by, ax*bx+ay*by);

                      aoc->endAngle = aoc->startAngle + range;
                      aoc->startAngle *= 180 / M_PI;
                      aoc->endAngle   *= 180 / M_PI;
                      result = aoc;
                }
              } break;
              case GeomAbs_Ellipse: {
                gp_Elips2d prjEllipse = curve.Ellipse();

                double f = curve.FirstParameter();
                double l = curve.LastParameter();
                gp_Pnt2d s = curve.Value(f);
                gp_Pnt2d e = curve.Value(l);

                if (fabs(l-f) > 1.0 && s.SquareDistance(e) < 0.001) {
                      Ellipse *ellipse = new Ellipse();
                      ellipse->major = prjEllipse.MajorRadius();
                      ellipse->minor = prjEllipse.MinorRadius();
                      ellipse->center = Base::Vector2D(prjEllipse.Location().X(),prjEllipse.Location().Y());
                      result = ellipse;
                } else {
                      // TODO implement this correctly
                      AOE *aoe = new AOE();
                      aoe->major = prjEllipse.MajorRadius();
                      aoe->minor = prjEllipse.MinorRadius();
                      aoe->center = Base::Vector2D(prjEllipse.Location().X(),prjEllipse.Location().Y());
                      result =  aoe;
                }
              } break;
              case GeomAbs_BSplineCurve: {
              } break;

            default:
              break;
        }

        return result;
    }
}

void GeometryObject::extractFaces(HLRBRep_Algo *myAlgo,
                                  const TopoDS_Shape &S,
                                  int type,
                                  bool visible,
                                  ExtractionType extractionType,
                                  std::vector<DrawingGeometry::Face *> &projFaces,
                                  std::vector<int> &faceRefs) const
{

    if(!myAlgo)
        return;

    Handle_HLRBRep_Data DS = myAlgo->DataStructure();
    if (DS.IsNull()) {
        Base::Console().Log("Drawing::GeometryObject::extractFaces - DS is Null\n");
        return;
    }

    DS->Projector().Scaled(true);

    int e1 = 1;
    int e2 = DS->NbEdges();
    int f1 = 1;
    int f2 = DS->NbFaces();

    if (!S.IsNull()) {
        Standard_Integer v1,v2;
        Standard_Integer index = myAlgo->Index(S);
        if(index == 0)  {
            Base::Console().Log("Drawing::GeometryObject::extractFaces - myAlgo->Index(S) == 0\n");
            return;
        }
        myAlgo->ShapeBounds(index).Bounds(v1,v2,e1,e2,f1,f2);
    }

    TopTools_IndexedMapOfShape anfIndices;
    TopTools_IndexedMapOfShape& Faces = DS->FaceMap();
    TopExp::MapShapes(S, TopAbs_FACE, anfIndices);

    BRep_Builder B;

    /* ----------------- Extract Faces ------------------ */
    for (int iface = f1; iface <= f2; iface++) {
        TopoDS_Shape face;
        B.MakeCompound(TopoDS::Compound(face));

        drawFace(visible,5,iface,DS,face);

        DrawingGeometry::Face *myFace = new DrawingGeometry::Face();

        createWire(face, myFace->wires);

        // Proces each wire
        projFaces.push_back(myFace);

        int idxFace;
        for (int i = 1; i <= anfIndices.Extent(); i++) {
            idxFace = Faces.FindIndex(anfIndices(iface));
            if (idxFace != 0) {
                break;
            }
        }

        if(idxFace == 0)
            idxFace = -1; // If Face not found - select hidden

        // Push the found face index onto references stack
        faceRefs.push_back(idxFace);
    }

    DS->Projector().Scaled(false);
}

bool GeometryObject::shouldDraw(const bool inFace, const int typ, HLRBRep_EdgeData& ed)
{
    bool todraw = false;
    if(inFace)
        todraw = true;
    else if (typ == 3)
        todraw = ed.Rg1Line() && !ed.RgNLine();
    else if (typ == 4)
        todraw = ed.RgNLine();
    else
        todraw =!ed.Rg1Line();

    return todraw;
}

void GeometryObject::extractVerts(HLRBRep_Algo *myAlgo, const TopoDS_Shape &S, HLRBRep_EdgeData& ed, int ie, ExtractionType extractionType)
{
    if(!myAlgo)
        return;

    Handle_HLRBRep_Data DS = myAlgo->DataStructure();

    if (DS.IsNull())
      return;

    DS->Projector().Scaled(true);

    TopTools_IndexedMapOfShape anIndices;
    TopTools_IndexedMapOfShape anvIndices;

    TopExp::MapShapes(S, TopAbs_EDGE, anIndices);
    TopExp::MapShapes(S, TopAbs_VERTEX, anvIndices);

    int edgeNum = anIndices.Extent();
    // Load the edge
    if(ie < 0) {

    } else {
        TopoDS_Shape shape = anIndices.FindKey(ie);
        TopoDS_Edge edge = TopoDS::Edge(shape);

        // Gather a list of points associated with this curve
        std::list<TopoDS_Shape> edgePoints;

        TopExp_Explorer xp;
        xp.Init(edge,TopAbs_VERTEX);
        while(xp.More()) {
            edgePoints.push_back(xp.Current());
            xp.Next();
        }
        for(std::list<TopoDS_Shape>::const_iterator it = edgePoints.begin(); it != edgePoints.end(); ++it) {

            // Should share topological data structure so can reference
            int iv = anvIndices.FindIndex(*it); // Index of the found vertex

            if(iv < 0)
                continue;

            // Check if vertex has already been addded
            std::vector<int>::iterator vert;
            vert = std::find(vertexReferences.begin(), vertexReferences.end(), iv);

            if(vert == vertexReferences.end()) {

                // If the index wasnt found and is unique, the point is projected using the HLR Projector Algorithm
                gp_Pnt2d prjPnt;
                DS->Projector().Project(BRep_Tool::Pnt(TopoDS::Vertex(*it)), prjPnt);

                // Check if this point lies on a visible section of the projected curve
                double sta,end;
                float tolsta,tolend;

                // There will be multiple edges that form the total edge so collect these
                BRep_Builder B;
                TopoDS_Compound comp;
                B.MakeCompound(comp);

                TopoDS_Edge E;
                HLRAlgo_EdgeIterator It;

                for(It.InitVisible(ed.Status()); It.MoreVisible(); It.NextVisible()) {
                    It.Visible(sta,tolsta,end,tolend);

                    E = HLRBRep::MakeEdge(ed.Geometry(),sta,end);
                    if (!E.IsNull()) {
                        B.Add(comp,E);
                    }
                }

                bool vertexVisible = false;
                TopExp_Explorer exp;
                exp.Init(comp,TopAbs_VERTEX);
                while(exp.More()) {

                    gp_Pnt pnt = BRep_Tool::Pnt(TopoDS::Vertex(exp.Current()));
                    gp_Pnt2d edgePnt(pnt.X(), pnt.Y());
                    if(edgePnt.SquareDistance(prjPnt) < Precision::Confusion()) {
                        vertexVisible = true;
                        break;
                    }
                    exp.Next();
                }

                if(vertexVisible) {
                    Vertex *myVert = new Vertex(prjPnt.X(), prjPnt.Y());
                    vertexGeom.push_back(myVert);
                    vertexReferences.push_back(iv);
                }
            }
        }
    }
}

void GeometryObject::extractEdges(HLRBRep_Algo *myAlgo, const TopoDS_Shape &S, int type, bool visible, ExtractionType extractionType)
{
    if (!myAlgo)
      return;

    Handle_HLRBRep_Data DS = myAlgo->DataStructure();

    if (DS.IsNull())
        return;

    DS->Projector().Scaled(true);

    int e1 = 1;
    int e2 = DS->NbEdges();
    int f1 = 1;
    int f2 = DS->NbFaces();

    if (!S.IsNull()) {
        int v1,v2;
        int index = myAlgo->Index(S);
        if(index == 0)
            return;
        myAlgo->ShapeBounds(index).Bounds(v1,v2,e1,e2,f1,f2);
    }

    HLRBRep_EdgeData* ed = &(DS->EDataArray().ChangeValue(e1 - 1));

    // Get map of edges and faces from projected geometry
    TopTools_IndexedMapOfShape& Edges = DS->EdgeMap();
    TopTools_IndexedMapOfShape anIndices;

    TopExp::MapShapes(S, TopAbs_EDGE, anIndices);

    for (int j = e1; j <= e2; j++) {
        ed++;
        if (ed->Selected() && !ed->Vertical()) {
            ed->Used(false);
            ed->HideCount(0);

        } else {
            ed->Used(true);
        }
    }

    BRep_Builder B;

    std::list<int> notFound;
    /* ----------------- Extract Edges ------------------ */
    for (int i = 1; i <= anIndices.Extent(); i++) {
        int ie = Edges.FindIndex(anIndices(i));
        if (ie != 0) {

            HLRBRep_EdgeData& ed = DS->EDataArray().ChangeValue(ie);
            if(!ed.Used()) {
                if(true) {

                    TopoDS_Shape result;
                    B.MakeCompound(TopoDS::Compound(result));

                    drawEdge(ed, result, visible);

                    // Extract and Project Vertices
                    extractVerts(myAlgo, S, ed, i, extractionType);

                    int edgesAdded = calculateGeometry(result, extractionType, edgeGeom);

                    // Push the edge references
                    while(edgesAdded--)
                        edgeReferences.push_back(i);
                }

                ed.Used(Standard_True);
            }
        } else {
            notFound.push_back(i);
        }
    }



    // Add any remaining edges that couldn't be found
    HLRBRep_EdgeData* edge = &(DS->EDataArray().ChangeValue(e1 - 1));
    int edgeIdx = -1; // Negative index for edge references
    for (int ie = e1; ie <= e2; ie++) {
      // Co
      HLRBRep_EdgeData& ed = DS->EDataArray().ChangeValue(ie);
      if (!ed.Used()) {
          if(shouldDraw(false, type, ed)) {
              const TopoDS_Shape &shp = Edges.FindKey(ie);

              //Compares original shape to see if match
              if(!shp.IsNull()) {
                  const TopoDS_Edge& edge = TopoDS::Edge(shp);
                  BRepAdaptor_Curve adapt1(edge);
                  for (std::list<int>::iterator it= notFound.begin(); it!= notFound.end(); ++it){
                      BRepAdaptor_Curve adapt2(TopoDS::Edge(anIndices(*it)));
                      if(isSameCurve(adapt1, adapt2)) {
                          edgeIdx = *it;
//                           notFound.erase(it);
                          break;
                      }
                  }
              }

              TopoDS_Shape result;
              B.MakeCompound(TopoDS::Compound(result));

              drawEdge(ed, result, visible);
              int edgesAdded = calculateGeometry(result, extractionType, edgeGeom);

              // Push the edge references
              while(edgesAdded--)
                  edgeReferences.push_back(edgeIdx);
          }
          ed.Used(true);
      }
    }

    DS->Projector().Scaled(false);
}

/**
 * Note projected edges are broken up so start and end parameters differ.
 */
bool GeometryObject::isSameCurve(const BRepAdaptor_Curve &c1, const BRepAdaptor_Curve &c2) const
{


    if(c1.GetType() != c2.GetType())
        return false;
#if 0
    const gp_Pnt& p1S = c1.Value(c1.FirstParameter());
    const gp_Pnt& p1E = c1.Value(c1.LastParameter());

    const gp_Pnt& p2S = c2.Value(c2.FirstParameter());
    const gp_Pnt& p2E = c2.Value(c2.LastParameter());

    bool state =  (p1S.IsEqual(p2S, Precision::Confusion()) && p1E.IsEqual(p2E, Precision::Confusion()));

    if( s ||
        (p1S.IsEqual(p2E, Precision::Confusion()) && p1E.IsEqual(p2S, Precision::Confusion())) ){
        switch(c1.GetType()) {
          case GeomAbs_Circle: {

                  gp_Circ circ1 = c1.Circle();
                  gp_Circ circ2 = c2.Circle();

                  const gp_Pnt& p = circ1.Location();
                  const gp_Pnt& p2 = circ2.Location();

                  double radius1 = circ1.Radius();
                  double radius2 = circ2.Radius();
                  double f1 = c1.FirstParameter();
                  double f2 = c2.FirstParameter();
                  double l1 = c1.LastParameter();
                  double l2 = c2.LastParameter();
                  c1.Curve().Curve()->
                  if( p.IsEqual(p2,Precision::Confusion()) &&
                  radius2 - radius1 < Precision::Confusion()) {
                      return true;
                  }
          } break;
          default: break;
        }
    }
#endif
    return false;
}

void GeometryObject::createWire(const TopoDS_Shape &input,
                                std::vector<DrawingGeometry::Wire *> &wires) const
{
    if (input.IsNull()) {
        Base::Console().Log("Drawing::GeometryObject::createWire input is NULL\n");
        return; // There is no OpenCascade Geometry to be calculated
    }

    std::list<TopoDS_Edge> edgeList;

    // Explore all edges of input and calculate base geometry representation
    TopExp_Explorer edges(input, TopAbs_EDGE);
    while (edges.More()) {
        edgeList.push_back(TopoDS::Edge(edges.Current()));
        edges.Next();
    }


    // Combine connected edges into wires.

    // BRepBuilderAPI_MakeWire has an annoying behaviour where the only [sane]
    // way to test whether an edge connects to a wire is to attempt adding
    // the edge.  But, if the last added edge was not connected to the wire,
    // BRepBuilderAPI_MakeWire::Wire() will throw an exception.  So, we need
    // to hang on to the last successfully added edge to "reset" mkWire.
    bool lastAddFailed;
    TopoDS_Edge lastGoodAdd;

    while (edgeList.size() > 0) {
        // add and erase first edge
        BRepBuilderAPI_MakeWire mkWire;
        mkWire.Add(edgeList.front());
        lastAddFailed = false;
        lastGoodAdd = edgeList.front();
        edgeList.pop_front();

        // try to connect each edge to the wire, the wire is complete if no more egdes are connectible
        bool found;
        do {
            found = false;
            for (std::list<TopoDS_Edge>::iterator pE = edgeList.begin(); pE != edgeList.end(); ++pE) {

                // Try adding edge - doesn't necessarily add it
                mkWire.Add(*pE);

                if (mkWire.Error() != BRepBuilderAPI_DisconnectedWire) {
                    // Edge added!  Remember it, 
                    lastAddFailed = false;
                    lastGoodAdd = *pE;

                    // ...remove it from edgeList,
                    edgeList.erase(pE);

                    // ...and start searching for the next edge
                    found = true;
                    break;
                } else {
                    lastAddFailed = true;
                }
            } 
        } while (found);

        // See note above re: BRepBuilderAPI_MakeWire annoying behaviour
        if (lastAddFailed) {
            mkWire.Add(lastGoodAdd);
        }

        if (mkWire.Error() == BRepBuilderAPI_WireDone) {
            DrawingGeometry::Wire *genWire = new DrawingGeometry::Wire();

            // If there are problems in this area, have a look at ShapeFix_Wire
            BRepTools_WireExplorer explr(mkWire.Wire());
            while (explr.More()) {
                BRep_Builder builder;
                TopoDS_Compound comp;
                builder.MakeCompound(comp);
                builder.Add(comp, explr.Current());

                calculateGeometry(comp, Plain, genWire->geoms);
                explr.Next();
            }
            wires.push_back(genWire);
        } else if(mkWire.Error() == BRepBuilderAPI_DisconnectedWire) {
            Standard_Failure::Raise("Fatal error occurred in GeometryObject::createWire()");
        }
    }
}

void GeometryObject::extractGeometry(const TopoDS_Shape &input, const Base::Vector3d &direction, bool extractHidden, const Base::Vector3d &xAxis)
{
    // Clear previous Geometry and References that may have been stored
    clear();

    ///TODO: Consider whether it would be possible/beneficial to cache some of this effort IR
    TopoDS_Shape transShape;
    HLRBRep_Algo *brep_hlr = NULL;
    try {
        // Find 3-dimensional bounding box around input
        Bnd_Box bounds;
        BRepBndLib::Add(input, bounds);
        bounds.SetGap(0.0);
        Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
        bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);

        // Note centre point of 3D object for...
        gp_Pnt inputCentre((xMin + xMax) / 2.0,
                           (yMin + yMax) / 2.0,
                           (zMin + zMax) / 2.0);

        // First - Making a copy of the shape that's scaled
        //         around the original shape's centre point
        gp_Trsf matScale;
        matScale.SetScale(inputCentre, Scale);

        BRepBuilderAPI_Transform mkTrf(invertY(input), matScale);
        transShape = mkTrf.Shape();

        brep_hlr = new HLRBRep_Algo();
        brep_hlr->Add(transShape);

        // Second - Projecting the shape into a properly-oriented
        //          space with the object's centroid at the origin.
        projXAxis = xAxis;
        projNorm = direction;
        gp_Ax2 transform;

        if(xAxis.Length() > FLT_EPSILON) {
            transform = gp_Ax2(inputCentre,
                               gp_Dir(direction.x, direction.y, direction.z),
                               gp_Dir(xAxis.x, xAxis.y, xAxis.z));
        } else {
            transform = gp_Ax2(inputCentre,
                               gp_Dir(direction.x, direction.y, direction.z));
        }
        HLRAlgo_Projector projector( transform );
        brep_hlr->Projector(projector);
        brep_hlr->Update();
        brep_hlr->Hide();
    }
    catch (...) {
        Standard_Failure::Raise("Fatal error occurred while projecting shape");
    }

    // extracting the result sets:

    //TODO: What is this? IR

    // V  = shapes.VCompound       ();// hard edge visibly
    // V1 = shapes.Rg1LineVCompound();// Smoth edges visibly
    // VN = shapes.RgNLineVCompound();// contour edges visibly
    // VO = shapes.OutLineVCompound();// contours apparents visibly
    // VI = shapes.IsoLineVCompound();// isoparamtriques   visibly
    // H  = shapes.HCompound       ();// hard edge       invisibly
    // H1 = shapes.Rg1LineHCompound();// Smoth edges  invisibly
    // HN = shapes.RgNLineHCompound();// contour edges invisibly
    // HO = shapes.OutLineHCompound();// contours apparents invisibly
    // HI = shapes.IsoLineHCompound();// isoparamtriques   invisibly

    // Extract Hidden Edges
    if(extractHidden)
        extractEdges(brep_hlr, transShape, 5, false, WithHidden);// Hard Edge
//     calculateGeometry(extractCompound(brep_hlr, invertShape, 2, false), WithHidden); // Outline
//     calculateGeometry(extractCompound(brep_hlr, invertShape, 3, false), (ExtractionType)(WithSmooth | WithHidden)); // Smooth

    // Extract Visible Edges
    extractEdges(brep_hlr, transShape, 5, true, WithSmooth);  // Hard Edge

//     calculateGeometry(extractCompound(brep_hlr, invertShape, 2, true), Plain);  // Outline
//     calculateGeometry(extractCompound(brep_hlr, invertShape, 3, true), WithSmooth); // Smooth Edge

    // Extract Faces
    extractFaces(brep_hlr, transShape, 5, true,WithSmooth,faceGeom,faceReferences);

    // House Keeping
    delete brep_hlr;
}


int GeometryObject::calculateGeometry(const TopoDS_Shape &input,
                                      const ExtractionType extractionType,
                                      std::vector<BaseGeom *> &geom) const
{
    if(input.IsNull()) {
        Base::Console().Log("Drawing::GeometryObject::calculateGeometry input is NULL\n");
        return 0; // There is no OpenCascade Geometry to be calculated
    }

    // build a mesh to explore the shape
    //BRepMesh::Mesh(input, this->Tolerance);   //OCC has removed BRepMesh::Mesh() as of v6.8.0.oce-0.17-dev
    BRepMesh_IncrementalMesh(input, this->Tolerance);

    int geomsAdded = 0;

    // Explore all edges of input and calculate base geometry representation
    TopExp_Explorer edges(input, TopAbs_EDGE);
    for (int i = 1 ; edges.More(); edges.Next(),i++) {
        const TopoDS_Edge& edge = TopoDS::Edge(edges.Current());
        BRepAdaptor_Curve adapt(edge);

        switch(adapt.GetType()) {
          case GeomAbs_Circle: {
            double f = adapt.FirstParameter();
            double l = adapt.LastParameter();
            gp_Pnt s = adapt.Value(f);
            gp_Pnt e = adapt.Value(l);

            if (fabs(l-f) > 1.0 && s.SquareDistance(e) < 0.001) {
                  Circle *circle = new Circle(adapt);
                  circle->extractType = extractionType;
                  geom.push_back(circle);
            } else {
                  AOC *aoc = new AOC(adapt);
                  aoc->extractType = extractionType;
                  geom.push_back(aoc);
            }
          } break;
          case GeomAbs_Ellipse: {
            double f = adapt.FirstParameter();
            double l = adapt.LastParameter();
            gp_Pnt s = adapt.Value(f);
            gp_Pnt e = adapt.Value(l);
            if (fabs(l-f) > 1.0 && s.SquareDistance(e) < 0.001) {
                  Ellipse *ellipse = new Ellipse(adapt);
                  ellipse->extractType = extractionType;
                  geom.push_back(ellipse);
            } else {
                  AOE *aoe = new AOE(adapt);
                  aoe->extractType = extractionType;
                  geom.push_back(aoe);
            }
          } break;
          case GeomAbs_BSplineCurve: {
            BSpline *bspline = 0;
            try {
                bspline = new BSpline(adapt);
                bspline->extractType = extractionType;
                geom.push_back(bspline);
                break;
            }
            catch (Standard_Failure) {
                delete bspline;
                bspline = 0;
                // Move onto generating a primitive
            }
          }
          default: {
            Generic *primitive = new Generic(adapt);
            primitive->extractType = extractionType;
            geom.push_back(primitive);
          }  break;
        }
        geomsAdded++;
    }
    return geomsAdded;
}

/***************************************************************************
 *   Copyright (c) 2012 Luke Parry <l.parry@warwick.ac.uk>                 *
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
# include <BRepAdaptor_Curve.hxx>
# include <Geom_Circle.hxx>
# include <gp_Circ.hxx>
# include <gp_Elips.hxx>
#endif

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
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
#include <gp_Vec.hxx>
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
#include <BRepMesh.hxx>

#include <BRepAdaptor_CompCurve.hxx>
#include <Handle_BRepAdaptor_HCompCurve.hxx>
#include <Approx_Curve3d.hxx>
#include <BRepAdaptor_HCurve.hxx>
#include <Handle_BRepAdaptor_HCurve.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Handle_Geom_BSplineCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <GeomConvert_BSplineCurveToBezierCurve.hxx>
#include <GeomConvert_BSplineCurveKnotSplitting.hxx>
#include <Geom2d_BSplineCurve.hxx>

#include <Base/Console.h>
#include <Base/Tools2D.h>
#include <Base/Vector3D.h>

#include "Geometry.h"

using namespace DrawingGeometry;

// Collection of Geometric Features
Wire::Wire()
{
  
}

Wire::~Wire()
{
    for(std::vector<BaseGeom *>::iterator it = this->geoms.begin(); it != this->geoms.end(); ++it) {
        delete (*it);
        *it = 0;
    }
    this->geoms.clear();
}

Face::Face()
{
  
}

Face::~Face()
{
    for(std::vector<Wire *>::iterator it = this->wires.begin(); it != this->wires.end(); ++it) {
        delete (*it);
        *it = 0;
    }
    this->wires.clear();
}

Ellipse::Ellipse()
{
    this->geomType = ELLIPSE;
}

Ellipse::Ellipse(const BRepAdaptor_Curve& c)
{
    this->geomType = ELLIPSE;

    gp_Elips ellp = c.Ellipse();
    const gp_Pnt& p = ellp.Location();

    this->center = Base::Vector2D(p.X(),p.Y());

    this->major = ellp.MajorRadius();
    this->minor = ellp.MinorRadius();

    gp_Dir xaxis = ellp.XAxis().Direction();
    this->angle = xaxis.AngleWithRef(gp_Dir(1,0,0),gp_Dir(0,0,-1));
    this->angle *= 180 / M_PI;
}

AOE::AOE()
{
    this->geomType = ARCOFELLIPSE;
}

AOE::AOE(const BRepAdaptor_Curve& c) : Ellipse(c)
{
    this->geomType = ARCOFELLIPSE;

    gp_Elips ellp = c.Ellipse();

    double f = c.FirstParameter();
    double l = c.LastParameter();
    gp_Pnt s = c.Value(f);
    gp_Pnt m = c.Value((l+f)/2.0);
    gp_Pnt e = c.Value(l);

    gp_Vec v1(m,s);
    gp_Vec v2(m,e);
    gp_Vec v3(0,0,1);
    double a = v3.DotCross(v1,v2);

    this->startAngle = f;
    this->endAngle = l;
    this->cw = (a < 0) ? true: false;
    this->largeArc = (l-f > M_PI) ? true : false;
    
    startPnt = Base::Vector2D(s.X(), s.Y());
    endPnt = Base::Vector2D(e.X(), e.Y());
    /*
            char las = (l-f > D_PI) ? '1' : '0'; // large-arc-flag
        char swp = (a < 0) ? '1' : '0'; // sweep-flag, i.e. clockwise (0) or counter-clockwise (1)
        out << "<path d=\"M" << s.X() <<  " " << s.Y()
            << " A" << r1 << " " << r2 << " "
            << angle << " " << las << " " << swp << " "
            << e.X() << " " << e.Y() << "\" />" << std::endl;
//     if (this->startAngle > this->endAngle) {// if arc is reversed
//         std::swap(this->startAngle, this->endAngle);
//     }*/
    
//     double ax = s.X() - this->center.fX;
//     double ay = s.Y() - this->center.fY;
//     double bx = e.X() - this->center.fX;
//     double by = e.Y() - this->center.fY;
    
//     this->startAngle = f;
//     float range = l-f;
// 
//     this->endAngle = startAngle + range;

    this->startAngle *= 180 / M_PI;
    this->endAngle   *= 180 / M_PI;
}

Circle::Circle()
{
      this->geomType = CIRCLE;
}

Circle::Circle(const BRepAdaptor_Curve& c)
{
    this->geomType = CIRCLE;

    gp_Circ circ = c.Circle();
    const gp_Pnt& p = circ.Location();

    this->radius = circ.Radius();
    this->center = Base::Vector2D(p.X(), p.Y());
}

AOC::AOC()
{
    this->geomType = ARCOFCIRCLE;
}

AOC::AOC(const BRepAdaptor_Curve& c) : Circle(c)
{
    this->geomType = ARCOFCIRCLE;
    
    double f = c.FirstParameter();
    double l = c.LastParameter();
    gp_Pnt s = c.Value(f);
    gp_Pnt m = c.Value((l+f)/2.0);
    gp_Pnt e = c.Value(l);

    gp_Vec v1(m,s);
    gp_Vec v2(m,e);
    gp_Vec v3(0,0,1);
    double a = v3.DotCross(v1,v2);

    this->startAngle = f;
    this->endAngle = l;
    this->cw = (a < 0) ? true: false;
    this->largeArc = (l-f > M_PI) ? true : false;
    
    startPnt = Base::Vector2D(s.X(), s.Y());
    endPnt = Base::Vector2D(e.X(), e.Y());
              
    this->startAngle *= 180 / M_PI;
    this->endAngle   *= 180 / M_PI;
}

Generic::Generic()
{
    this->geomType = GENERIC;
}

Generic::Generic(const BRepAdaptor_Curve& c)
{
    this->geomType = GENERIC;

    TopLoc_Location location;
    Handle_Poly_Polygon3D polygon = BRep_Tool::Polygon3D(c.Edge(), location);
    if (!polygon.IsNull()) {
        const TColgp_Array1OfPnt &nodes = polygon->Nodes();
        for (int i = nodes.Lower(); i <= nodes.Upper(); i++){
             this->points.push_back(Base::Vector2D(nodes(i).X(), nodes(i).Y()));
        }
    }
}

BSpline::BSpline()
{
    this->geomType = BSPLINE;
}

BSpline::BSpline(const BRepAdaptor_Curve& c)
{
    this->geomType = BSPLINE;
    Handle_Geom_BSplineCurve spline = c.BSpline();
    if (spline->Degree() > 3) {
        Standard_Real tol3D = 0.001;
        Standard_Integer maxDegree = 3, maxSegment = 10;
        Handle_BRepAdaptor_HCurve hCurve = new BRepAdaptor_HCurve(c);
        // approximate the curve using a tolerance
        Approx_Curve3d approx(hCurve,tol3D,GeomAbs_C2, maxSegment, maxDegree);
        if (approx.IsDone() && approx.HasResult()) {
            // have the result
            spline = approx.Curve();
        }
    }

    GeomConvert_BSplineCurveToBezierCurve crt(spline);
    Standard_Integer arcs = crt.NbArcs();
    for (Standard_Integer i = 1; i <= arcs; i++) {
        Handle_Geom_BezierCurve bezier = crt.Arc(i);
        Standard_Integer poles = bezier->NbPoles();
        BezierSegment segment;
        if (bezier->Degree() == 3) {
            if (poles != 4)
                Standard_Failure::Raise("do it the generic way");
            segment.poles = 4;
            gp_Pnt p1 = bezier->Pole(1);
            gp_Pnt p2 = bezier->Pole(2);
            gp_Pnt p3 = bezier->Pole(3);
            gp_Pnt p4 = bezier->Pole(4);
            if (i == 1) {
                segment.pnts[0] = Base::Vector2D(p1.X(), p1.Y());
                segment.pnts[1] = Base::Vector2D(p2.X(), p2.Y());
                segment.pnts[2] = Base::Vector2D(p3.X(), p3.Y());
                segment.pnts[3] = Base::Vector2D(p4.X(), p4.Y());
            } else {
                segment.pnts[2] = Base::Vector2D(p3.X(), p3.Y());
                segment.pnts[3] = Base::Vector2D(p4.X(), p4.Y());
            }
        } else if (bezier->Degree() == 2) {
            if (poles != 3)
                Standard_Failure::Raise("do it the generic way");
            segment.poles = 3;
            gp_Pnt p1 = bezier->Pole(1);
            gp_Pnt p2 = bezier->Pole(2);
            gp_Pnt p3 = bezier->Pole(3);
            if(i == 1) {
                segment.pnts[0] = Base::Vector2D(p1.X(), p1.Y());
                segment.pnts[1] = Base::Vector2D(p2.X(), p2.Y());
                segment.pnts[2] = Base::Vector2D(p3.X(), p3.Y());
            } else {
                segment.pnts[2] = Base::Vector2D(p3.X(), p3.Y());
            }
        } else {
            Standard_Failure::Raise("do it the generic way");
        }
        this->segments.push_back(segment);
    }    
}
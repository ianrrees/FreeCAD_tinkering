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
#include <Base/Exception.h>
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
    for(std::vector<BaseGeom *>::iterator it = geoms.begin(); it != geoms.end(); ++it) {
        delete (*it);
        *it = 0;
    }
    geoms.clear();
}

Face::Face()
{
  
}

Face::~Face()
{
    for(std::vector<Wire *>::iterator it = wires.begin(); it != wires.end(); ++it) {
        delete (*it);
        *it = 0;
    }
    wires.clear();
}

Ellipse::Ellipse()
{
    geomType = ELLIPSE;
}

Ellipse::Ellipse(const BRepAdaptor_Curve& c)
{
    geomType = ELLIPSE;

    gp_Elips ellp = c.Ellipse();
    const gp_Pnt &p = ellp.Location();

    center = Base::Vector2D(p.X(), p.Y());

    major = ellp.MajorRadius();
    minor = ellp.MinorRadius();

    gp_Dir xaxis = ellp.XAxis().Direction();
    angle = xaxis.AngleWithRef(gp_Dir(1, 0, 0), gp_Dir(0, 0, -1));
    angle *= 180 / M_PI;
}

AOE::AOE()
{
    geomType = ARCOFELLIPSE;
}

AOE::AOE(const BRepAdaptor_Curve& c) : Ellipse(c)
{
    geomType = ARCOFELLIPSE;

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

    startAngle = f;
    endAngle = l;
    cw = (a < 0) ? true: false;
    largeArc = (l-f > M_PI) ? true : false;
    
    startPnt = Base::Vector2D(s.X(), s.Y());
    endPnt = Base::Vector2D(e.X(), e.Y());
    /*
            char las = (l-f > D_PI) ? '1' : '0'; // large-arc-flag
        char swp = (a < 0) ? '1' : '0'; // sweep-flag, i.e. clockwise (0) or counter-clockwise (1)
        out << "<path d=\"M" << s.X() <<  " " << s.Y()
            << " A" << r1 << " " << r2 << " "
            << angle << " " << las << " " << swp << " "
            << e.X() << " " << e.Y() << "\" />" << std::endl;
//     if (startAngle > endAngle) {// if arc is reversed
//         std::swap(startAngle, endAngle);
//     }*/
    
//     double ax = s.X() - center.fX;
//     double ay = s.Y() - center.fY;
//     double bx = e.X() - center.fX;
//     double by = e.Y() - center.fY;
    
//     startAngle = f;
//     float range = l-f;
// 
//     endAngle = startAngle + range;

    startAngle *= 180 / M_PI;
    endAngle   *= 180 / M_PI;
}

Circle::Circle()
{
    geomType = CIRCLE;
}

Circle::Circle(const BRepAdaptor_Curve& c)
{
    geomType = CIRCLE;

    gp_Circ circ = c.Circle();
    const gp_Pnt& p = circ.Location();

    radius = circ.Radius();
    center = Base::Vector2D(p.X(), p.Y());
}

AOC::AOC()
{
    geomType = ARCOFCIRCLE;
}

AOC::AOC(const BRepAdaptor_Curve& c) : Circle(c)
{
    geomType = ARCOFCIRCLE;
    
    double f = c.FirstParameter();
    double l = c.LastParameter();
    gp_Pnt s = c.Value(f);
    gp_Pnt m = c.Value((l+f)/2.0);
    gp_Pnt e = c.Value(l);

    gp_Vec v1(m,s);
    gp_Vec v2(m,e);
    gp_Vec v3(0,0,1);
    double a = v3.DotCross(v1,v2);

    startAngle = f;
    endAngle = l;
    cw = (a < 0) ? true: false;
    largeArc = (l-f > M_PI) ? true : false;
    
    startPnt = Base::Vector2D(s.X(), s.Y());
    endPnt = Base::Vector2D(e.X(), e.Y());
              
    startAngle *= 180 / M_PI;
    endAngle   *= 180 / M_PI;
}

Generic::Generic()
{
    geomType = GENERIC;
}

Generic::Generic(const BRepAdaptor_Curve& c)
{
    geomType = GENERIC;

    TopLoc_Location location;
    Handle_Poly_Polygon3D polygon = BRep_Tool::Polygon3D(c.Edge(), location);
    if (!polygon.IsNull()) {
        const TColgp_Array1OfPnt &nodes = polygon->Nodes();
        for (int i = nodes.Lower(); i <= nodes.Upper(); i++){
            points.push_back(Base::Vector2D(nodes(i).X(), nodes(i).Y()));
        }
    }
}

BSpline::BSpline()
{
    geomType = BSPLINE;
}

BSpline::BSpline(const BRepAdaptor_Curve &c)
{
    geomType = BSPLINE;
    Handle_Geom_BSplineCurve spline = c.BSpline();
    if (spline->Degree() > 3) {
        Standard_Real tol3D = 0.001;
        Standard_Integer maxDegree = 3, maxSegment = 10;
        Handle_BRepAdaptor_HCurve hCurve = new BRepAdaptor_HCurve(c);
        // approximate the curve using a tolerance
        //Approx_Curve3d approx(hCurve, tol3D, GeomAbs_C2, maxSegment, maxDegree);   //gives degree == 5  ==> too many poles ==> buffer overrun
        Approx_Curve3d approx(hCurve, tol3D, GeomAbs_C0, maxSegment, maxDegree);
        if (approx.IsDone() && approx.HasResult()) {
            spline = approx.Curve();
        } else {
            throw Base::Exception("Geometry::BSpline - could not approximate curve");
        }
    }

    GeomConvert_BSplineCurveToBezierCurve crt(spline);

    BezierSegment tempSegment;
    gp_Pnt controlPoint;

    for (Standard_Integer i = 1; i <= crt.NbArcs(); ++i) {
        Handle_Geom_BezierCurve bezier = crt.Arc(i);
        if (bezier->Degree() > 3) {
            throw Base::Exception("Geometry::BSpline - converted curve degree > 3");
        }
        tempSegment.poles = bezier->NbPoles();
        // Note: We really only need to keep the pnts[0] for the first Bezier segment,
        // assuming this only gets used as in QGraphicsItemViewPart::drawPainterPath
        for (int pole = 1; pole <= tempSegment.poles; ++pole) {
            controlPoint = bezier->Pole(pole);
            tempSegment.pnts[pole - 1] = Base::Vector2D(controlPoint.X(), controlPoint.Y());
        }
        segments.push_back(tempSegment);
    }    
}


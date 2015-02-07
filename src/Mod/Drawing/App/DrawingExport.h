/***************************************************************************
 *   Copyright (c) 2011 Werner Mayer <wmayer[at]users.sourceforge.net>     *
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


#ifndef DRAWING_EXPORT_H
#define DRAWING_EXPORT_H

#include <string>
#include <TopoDS_Edge.hxx>
#include <boost/concept_check.hpp>
#include <QString>

class TopoDS_Shape;
class BRepAdaptor_Curve;

namespace Base {
  class Vector2D;
}

namespace DrawingGeometry
{
  class BaseGeom;
}

namespace Drawing
{
  class FeaturePage;
  class FeatureTemplate;
  class FeatureView;
  class FeatureDimension;
}

namespace Drawing
{

class DrawingExport DrawingOutput
{
public:
    // If the curve is approximately a circle it will be returned,
    // otherwise a null edge is returned.
    TopoDS_Edge asCircle(const BRepAdaptor_Curve&) const;
    TopoDS_Edge asBSpline(const BRepAdaptor_Curve&, int maxDegree) const;
};

class DrawingExport SVGOutput : public DrawingOutput
{
public:
    SVGOutput();
    std::string exportEdges(const TopoDS_Shape&);

private:
    void printCircle(const BRepAdaptor_Curve&, std::ostream&);
    void printEllipse(const BRepAdaptor_Curve&, int id, std::ostream&);
    void printBSpline(const BRepAdaptor_Curve&, int id, std::ostream&);
    void printBezier(const BRepAdaptor_Curve&, int id, std::ostream&);
    void printGeneric(const BRepAdaptor_Curve&, int id, std::ostream&);
};


class DrawingExport Exporter
{
public:

    Exporter();
    ~Exporter();

    enum ExportMode{
        EXPORT_SVG,
        EXPORT_DXF
    };

    void setPage(Drawing::FeaturePage *page);
    void setExportType(ExportMode &mode );

    void exportDrawing();

    QString getResult();

protected:
    ExportMode mode;
    Drawing::FeaturePage *page;
    Drawing::Exporter *exporter;
};

class DrawingExport SVGExporter: public Exporter
{
public:
    SVGExporter();
    ~SVGExporter();

    std::string exportEdges(const TopoDS_Shape&);

    void run();

protected:
    void printTemplate(const Drawing::FeatureTemplate *) const;
    void printDimension(const Drawing::FeatureDimension *) const;
    void printView(const Drawing::FeatureView *) const;

private:
    void printCircle(const DrawingGeometry::BaseGeom *, QString &str);
    void printEllipse(const DrawingGeometry::BaseGeom *, QString &str);
    void printBSpline(const DrawingGeometry::BaseGeom *, QString &str);
    void printGeneric(const DrawingGeometry::BaseGeom *, QString &str);
};



/* dxf output section - Dan Falck 2011/09/25  */
class DrawingExport DXFOutput : public DrawingOutput
{
public:
    DXFOutput();
    std::string exportEdges(const TopoDS_Shape&);

private:
    void printHeader(std::ostream& out);
    void printCircle(const BRepAdaptor_Curve&, std::ostream&);
    void printEllipse(const BRepAdaptor_Curve&, int id, std::ostream&);
    void printBSpline(const BRepAdaptor_Curve&, int id, std::ostream&);
    void printGeneric(const BRepAdaptor_Curve&, int id, std::ostream&);
};

} //namespace Drawing

#endif // DRAWING_EXPORT_H

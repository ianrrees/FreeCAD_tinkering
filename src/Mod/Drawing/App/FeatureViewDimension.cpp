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

#include "PreCompiled.h"

#ifndef _PreComp_
# include <sstream>
# include <cstring>
# include <QString>
# include <QStringList>
# include <QRegExp>
#endif

#include <App/Application.h>
#include <Base/Console.h>
#include <Base/Exception.h>
#include <Base/Parameter.h>

#include <Mod/Measure/App/Measurement.h>

#include "FeatureViewPart.h"
#include "FeatureViewDimension.h"

using namespace Drawing;

//===========================================================================
// FeatureViewDimension
//===========================================================================

PROPERTY_SOURCE(Drawing::FeatureViewDimension, Drawing::FeatureView)

const char* FeatureViewDimension::TypeEnums[]= {"Distance",
                                                "DistanceX",
                                                "DistanceY",
                                                "DistanceZ",
                                                "Radius",
                                                "Diameter",
                                                "Angle",
                                                NULL};

const char* FeatureViewDimension::ProjTypeEnums[]= {"True",
                                                    "Projected",
                                                    NULL};

FeatureViewDimension::FeatureViewDimension(void)
{

    Base::Reference<ParameterGrp> hGrp = App::GetApplication().GetUserParameter()
                                         .GetGroup("BaseApp")->GetGroup("Preferences")->GetGroup("Mod/Drawing");
    std::string fontName = hGrp->GetASCII("LabelFont", "osifont");

    ADD_PROPERTY_TYPE(References,(0,0),"Dimension",(App::PropertyType)(App::Prop_None),"Dimension Supporting References");
    ADD_PROPERTY_TYPE(Precision,(2)   ,"Dimension",(App::PropertyType)(App::Prop_None),"Dimension Precision");
    ADD_PROPERTY_TYPE(Font ,(fontName.c_str()),"Dimension",App::Prop_None, "The name of the font to use");
    ADD_PROPERTY_TYPE(Fontsize,(6)    ,"Dimension",(App::PropertyType)(App::Prop_None),"Dimension Font Size");
    ADD_PROPERTY_TYPE(CentreLines,(0) ,"Dimension",(App::PropertyType)(App::Prop_None),"Dimension Center Lines");
    ADD_PROPERTY_TYPE(ProjDirection ,(0.,0.,1.0), "Dimension",App::Prop_None,"Projection normal direction");
    ADD_PROPERTY_TYPE(FormatSpec,("%value%") ,"Dimension",(App::PropertyType)(App::Prop_None),"Dimension Format");

    Type.setEnums(TypeEnums);
    ADD_PROPERTY(Type,((long)0));

    ProjectionType.setEnums(ProjTypeEnums);
    ADD_PROPERTY(ProjectionType,((long)0));
    
    //hide the FeatureView properties that don't apply to Dimensions
    //App::PropertyType propType = static_cast<App::PropertyType>(App::Prop_Hidden|App::Prop_Output);
    int bitReadOnly = 2;
    int bitHidden = 3;
    ScaleType.StatusBits.set(bitReadOnly, true);
    ScaleType.StatusBits.set(bitHidden, true);
    Scale.StatusBits.set(bitReadOnly, true);
    Scale.StatusBits.set(bitHidden,true);
    Rotation.StatusBits.set(bitReadOnly, true);
    Rotation.StatusBits.set(bitHidden, true);
    //TODO: hide Dimension X,Y? 

    this->measurement = new Measure::Measurement();
}

FeatureViewDimension::~FeatureViewDimension()
{
    delete measurement;
    measurement = 0;
}

void FeatureViewDimension::onChanged(const App::Property* prop)
{
    if(prop == &References  ||
       prop == &Precision   ||
       prop == &Font        ||
       prop == &Fontsize    ||
       prop == &CentreLines ||
       prop == &ProjectionType ||
       prop == &FormatSpec) {
        this->touch();
    }
}

short FeatureViewDimension::mustExecute() const
{
    if(References.isTouched() ||
       Type.isTouched() ||
       ProjectionType.isTouched()
    )
        return 1;
    else
        return 0;
}

App::DocumentObjectExecReturn *FeatureViewDimension::execute(void)
{
    //Clear the previous measurement made
    measurement->clear();

    //Relcalculate the measurement based on references stored.
    const std::vector<App::DocumentObject*> &objects = References.getValues();
    const std::vector<std::string> &subElements = References.getSubValues();

    std::vector<App::DocumentObject*>::const_iterator obj = objects.begin();
    std::vector<std::string>::const_iterator subEl = subElements.begin();

    for(; obj != objects.end(); ++obj, ++subEl) {
        Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart *>(*obj);

        //Overall assumption is that the dimensions are only allowed for one view
        ProjDirection.setValue(viewPart->Direction.getValue());
        XAxisDirection.setValue(viewPart->XAxisDirection.getValue());
        App::DocumentObject *docObj = viewPart->Source.getValue();

        measurement->addReference(docObj,(*subEl).c_str());
    }

    return App::DocumentObject::StdReturn;
}

std::string  FeatureViewDimension::getFormatedValue() const
{
    QString str = QString::fromStdString(FormatSpec.getStrValue());

    QRegExp rx(QString::fromAscii("%(\\w+)%"));                        //any word bracketed by %
    QStringList list;
    int pos = 0;

    while ((pos = rx.indexIn(str, pos)) != -1) {
        list << rx.cap(0);
        pos += rx.matchedLength();
    }

    for(QStringList::const_iterator it = list.begin(); it != list.end(); ++it) {
        if(*it == QString::fromAscii("%value%")){
            double val = std::abs(getDimValue());
            str.replace(*it, QString::number(val, 'f', Precision.getValue()) );
        } else {                                                       //insert new placeholder replacement logic here
            str.replace(*it, QString::fromAscii(""));
        }
    }

    return str.toStdString();
}

double FeatureViewDimension::getDimValue() const
{
    const char *dimType = Type.getValueAsString();
    if(strcmp(ProjectionType.getValueAsString(), "True") == 0) {
        // True Values
        if(strcmp(dimType, "Distance") == 0) {
            return measurement->length();
        } else if(strcmp(dimType, "DistanceX") == 0){
            Base::Vector3d delta = measurement->delta();
            return delta.x;
        } else if(strcmp(dimType, "DistanceY") == 0){
            Base::Vector3d delta = measurement->delta();
            return delta.y;
        } else if(strcmp(dimType, "DistanceZ") == 0){
            Base::Vector3d delta = measurement->delta();
            return delta.z;
        } else if(strcmp(dimType, "Radius") == 0){
            return measurement->radius();
        } else if(strcmp(dimType, "Diameter") == 0){
            return measurement->radius() * 2.0;
        } else if(strcmp(dimType, "Angle") == 0){
            return measurement->angle();
        }
        throw Base::Exception("Unknown Dimension Type");
    } else {
        // Projected Values
        if(strcmp(dimType, "Distance") == 0 ||
            strcmp(dimType, "DistanceX") == 0 ||
            strcmp(dimType, "DistanceY") == 0 ||
            strcmp(dimType, "DistanceZ") == 0) {

            Base::Vector3d delta   = measurement->delta();
            Base::Vector3d projDir = ProjDirection.getValue();

            Base::Vector3d projDim = delta.ProjToPlane(Base::Vector3d(0.,0.,0.),
                                                       Base::Vector3d(projDir.x, projDir.y, projDir.z));

            Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart * >(References.getValues().at(0));

            Base::Vector3d xaxis = XAxisDirection.getValue();

            Base::Vector3d yaxis(projDir.y * xaxis.z - projDir.z * xaxis.y,
                                 projDir.z * xaxis.x - projDir.x * xaxis.z,
                                 projDir.x * xaxis.y - projDir.y * xaxis.x);

            // TODO not sure if this is a good idea to do i.e. works in all cases
            // Argument that 2D projection is always orientated the same way
            double x, y;
            x = projDim.x * xaxis.x + projDim.y * xaxis.y + projDim.z * xaxis.z;
            y = projDim.x * yaxis.x + projDim.y * yaxis.y + projDim.z * yaxis.z;

            //Base::Console().Log("proj <%f %f %f>", delta.x, delta.y, delta.z);
            //Base::Console().Log("yaxis <%f %f %f>", yaxis.x, yaxis.y, yaxis.z);
            //Base::Console().Log("proj <%f %f %f>", projDim.x, projDim.y, projDim.z);
            if(strcmp(dimType, "Distance") == 0) {
                return projDim.Length();
            } else if(strcmp(dimType, "DistanceX") == 0) {
                return x;
            } else if(strcmp(dimType, "DistanceY") == 0) {
                return y;
            } else if(strcmp(dimType, "DistanceZ") == 0) {
                throw Base::Exception("Cannot use z direction for projection type");
            }
        } else if(strcmp(dimType, "Radius") == 0){
            return measurement->radius(); // Can only use true value
        } else if(strcmp(dimType, "Diameter") == 0){
            return measurement->radius() * 2.0; // Can only use true value
        } else if(strcmp(dimType, "Angle") == 0){

            // Must project lines to 2D so cannot use measurement framework this time
            //Relcalculate the measurement based on references stored.
            const std::vector<App::DocumentObject*> &objects = References.getValues();
            const std::vector<std::string> &subElements      = References.getSubValues();

            std::vector<App::DocumentObject*>::const_iterator obj = objects.begin();
            std::vector<std::string>::const_iterator subEl = subElements.begin();

            if(subElements.size() != 2) {
                throw Base::Exception("Two references required for angle measurement");
            }

            DrawingGeometry::BaseGeom * projGeoms[2];
            for(int i = 0; obj != objects.end(); ++obj, ++subEl, i++) {
                Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart *>(*obj);

                int idx;
                if((*subEl).substr(0,4) == "Edge") {
                    idx = std::atoi((*subEl).substr(4,4000).c_str());
                }

                projGeoms[i] = viewPart->getCompleteEdge(idx);
            }

            // Only can find angles with straight line edges
            if(projGeoms[0]->geomType == DrawingGeometry::GENERIC &&
               projGeoms[1]->geomType == DrawingGeometry::GENERIC) {
                DrawingGeometry::Generic *gen1 = static_cast<DrawingGeometry::Generic *>(projGeoms[0]);
                DrawingGeometry::Generic *gen2 = static_cast<DrawingGeometry::Generic *>(projGeoms[1]);

                Base::Vector3d p1S(gen1->points.at(0).fX, gen1->points.at(0).fY, 0.);
                Base::Vector3d p1E(gen1->points.at(1).fX, gen1->points.at(1).fY, 0.);

                Base::Vector3d p2S(gen2->points.at(0).fX, gen2->points.at(0).fY, 0.);
                Base::Vector3d p2E(gen2->points.at(1).fX, gen2->points.at(1).fY, 0.);

                Base::Vector3d dir1 = p1E - p1S;
                Base::Vector3d dir2 = p2E - p2S;

                // Line Intersetion (taken from ViewProviderSketch.cpp)
                double det = dir1.x*dir2.y - dir1.y*dir2.x;
                if ((det > 0 ? det : -det) < 1e-10)
                    throw Base::Exception("Invalid selection - Det = 0");

                double c1 = dir1.y*gen1->points.at(0).fX - dir1.x*gen1->points.at(0).fY;
                double c2 = dir2.y*gen2->points.at(1).fX - dir2.x*gen2->points.at(1).fY;
                double x = (dir1.x*c2 - dir2.x*c1)/det;
                double y = (dir1.y*c2 - dir2.y*c1)/det;

                // Intersection point
                Base::Vector3d p0 = Base::Vector3d(x,y,0);

                Base::Vector3d lPos((double) X.getValue(), (double) Y.getValue(), 0.);
                Base::Vector3d delta = lPos - p0;

                double angle = lPos.GetAngle(delta);

                // Create vectors point towards intersection always
                Base::Vector3d a = -p0, b = -p0;
                a += ((p1S - p0).Length() < FLT_EPSILON) ? p1E : p1S;
                b += ((p2S - p0).Length() < FLT_EPSILON) ? p2E : p2S;

                double angle2 = atan2( a.x*b.y - a.y*b.x, a.x*b.x + a.y*b.y );
                return angle2 * 180. / M_PI;
            } else {
                throw Base::Exception("Invalid selection");
            }
        }
    }
}


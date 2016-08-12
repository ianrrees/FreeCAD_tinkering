/***************************************************************************
 *   Copyright (c) 2016 Eivind Kvedalen <eivind@kvedalen.name>             *
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

#include <Base/Quantity.h>
#include <Base/QuantityPy.h>
#include <Base/Tools.h>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>

#include "MaterialDatabase.h"
#include "MaterialSource.h"

using namespace App;

TYPESYSTEM_SOURCE(App::MaterialDatabase, Base::BaseClass);

std::map<std::string, MaterialDatabase::PropertyInfo> MaterialDatabase::propertyMap;
std::map<int, std::string> MaterialDatabase::propertyIdMap;

/* Color */

std::string MaterialDatabase::color_any2str(const boost::any & data)
{
    std::stringstream s;
    const Color & color = boost::any_cast<Color>(data);

    s << "(" << color.r << " " << color.g << " " << color.b << " " << color.a << ")";

    return s.str();
}

boost::any MaterialDatabase::color_str2any(const std::string & data)
{
    static const std::string f = "([0-9]+?.[0-9]+|[0-9]+)";
    static const std::string s = "\\(\\s* " + f + "\\s+" + f + "\\s+" + f + "\\s+" + f + "\\s*\\)";
    static const boost::regex e(s);
    boost::cmatch cm;

    if (boost::regex_match(data.c_str(), cm, e)) {
        float r = atof(cm[1].str().c_str()),
                g = atof(cm[2].str().c_str()),
                b = atof(cm[3].str().c_str()),
                a = atof(cm[4].str().c_str());

        return Color(r, g, b, a);
    }
    else
        throw Base::Exception(str(boost::format("Invalid value: %1%") % data));
}

PyObject * MaterialDatabase::color_any2python(const boost::any & data)
{
    const Color & color = boost::any_cast<Color>(data);

    return Py_BuildValue("ffff", color.r, color.g, color.b, color.a);
}

boost::any MaterialDatabase::color_python2any(const PyObject * data)
{
    float r, g, b, a;

    if (PyArg_ParseTuple(const_cast<PyObject*>(data), "ffff", &r, &g, &b, &a))
        return Color(r, g, b, a);
    else if (PyArg_ParseTuple(const_cast<PyObject*>(data), "fff", &r, &g, &b))
        return Color(r, g, b);
    else
        throw Base::Exception("Failed to convert python object.");
}

/* Floats */

std::string MaterialDatabase::any2float(const boost::any & data)
{
    std::stringstream s;

    s << boost::any_cast<float>(data);

    return s.str();
}

boost::any MaterialDatabase::float2any(const std::string & data)
{
    char * end;
    float value;

    value = strtof(data.c_str(), &end);

    return value;
}

PyObject * MaterialDatabase::float2python(const boost::any & data)
{
    return Py_BuildValue("%f", boost::any_cast<float>(data));
}

boost::any MaterialDatabase::python2float(const PyObject * data)
{
    float value;

    if (PyArg_ParseTuple(const_cast<PyObject*>(data), "f", &value))
        return value;
    else
        throw Base::Exception("Failed to convert python object.");
}

/* Strings */

std::string MaterialDatabase::any2string(const boost::any & data)
{
    return boost::any_cast<std::string>(data);
}

boost::any MaterialDatabase::string2any(const std::string & data)
{
    return data;
}

PyObject * MaterialDatabase::string2python(const boost::any & data)
{
    return PyUnicode_FromString(boost::any_cast<std::string>(data).c_str());
}

boost::any MaterialDatabase::python2string(const PyObject * data)
{
    PyObject* unicode = PyUnicode_AsUTF8String(const_cast<PyObject*>(data));
    std::string string = PyString_AsString(unicode);
    Py_DECREF(unicode);

    return string;
}

/* Density */

std::string MaterialDatabase::any2density(const boost::any & data)
{
    std::stringstream s;
    const Base::Quantity & value(boost::any_cast<Base::Quantity>(data));

    s << value.getValue() * 1e9 << " kg/m^3";

    return s.str();
}

boost::any MaterialDatabase::density2any(const std::string & data)
{
    try {
        Base::Quantity value(Base::Quantity::parse(Base::Tools::fromStdString(data)));

        if (value.getUnit().isEmpty())
            value.setUnit(Base::Unit::Density);
        else if (value.getUnit() != Base::Unit::Density)
            throw Base::Exception(str(boost::format("Invalid unit for density %1%") % data));

        return value;
    }
    catch (Base::Exception & e) {
        throw Base::Exception(str(boost::format("Failed to parse density quantity %1%: %2%") % data % e.what()));
    }
}

PyObject * MaterialDatabase::density2python(const boost::any & data)
{
    return new Base::QuantityPy(new Base::Quantity(boost::any_cast<Base::Quantity>(data)));
}

boost::any MaterialDatabase::python2density(const PyObject * data)
{
    return *static_cast<const Base::QuantityPy*>(data)->getQuantityPtr();
}

/* Pressure */

std::string MaterialDatabase::any2pressure(const boost::any & data)
{
    std::stringstream s;
    const Base::Quantity & value(boost::any_cast<Base::Quantity>(data));

    s << value.getValue() / 1e6 << " MPa";

    return s.str();
}

boost::any MaterialDatabase::pressure2any(const std::string & data)
{
    try {
        Base::Quantity value(Base::Quantity::parse(Base::Tools::fromStdString(data)));

        if (value.getUnit().isEmpty()) {
            value.setUnit(Base::Unit::Pressure);
            value = value * Base::Quantity(1000000); // Default to MPa if no unit given
        }
        else if (value.getUnit() != Base::Unit::Pressure)
            throw Base::Exception(str(boost::format("Invalid unit for pressure: %1%") % data));

        return value;
    }
    catch (Base::Exception & e) {
        throw Base::Exception(str(boost::format("Failed to parse pressure quantity %1%: %2%") % data % e.what()));
    }
}

PyObject * MaterialDatabase::pressure2python(const boost::any & data)
{
    return new Base::QuantityPy(new Base::Quantity(boost::any_cast<Base::Quantity>(data)));
}

boost::any MaterialDatabase::python2pressure(const PyObject * data)
{
    return *static_cast<const Base::QuantityPy*>(data)->getQuantityPtr();
}


MaterialDatabase::MaterialDatabase(MaterialDatabase * _parent)
    : parent(_parent)
{
    /* General properties */

    registerProperty("Name", string2any, any2string, python2string, string2python);
    registerProperty("Father", string2any, any2string, python2string, string2python);
    registerProperty("Description", string2any, any2string, python2string, string2python);
    registerProperty("SpecificWeight", density2any, any2density, python2density, density2python);
    registerProperty("Vendor", string2any, any2string, python2string, string2python);
    registerProperty("ProductURL", string2any, any2string, python2string, string2python);
    registerProperty("SpecificPrice", float2any, any2float, python2float, float2python);

    /* Mechanical */
    registerProperty("YoungsModulus", pressure2any, any2pressure, python2pressure, pressure2python);
    registerProperty("UltimateTensileStrength", pressure2any, any2pressure, python2pressure, pressure2python);
    registerProperty("Hardness", string2any, any2string, python2string, string2python);
    registerProperty("EN-10027-1", string2any, any2string, python2string, string2python);

    registerProperty("PoissonRatio", float2any, any2float, python2float, float2python);
    registerProperty("Density", density2any, any2density, python2density, density2python);
    registerProperty("KindOfMaterial", string2any, any2string, python2string, string2python);
    registerProperty("MaterialNumber", float2any, any2float, python2float, float2python);
    registerProperty("ModulusOfShare", float2any, any2float, python2float, float2python);
    registerProperty("Norm", string2any, any2string, python2string, string2python);
    registerProperty("ThermalExpansionCoefficient", float2any, any2float, python2float, float2python);
    registerProperty("UltimateStrain", pressure2any, any2pressure, python2pressure, pressure2python);
    registerProperty("YieldStrength", pressure2any, any2pressure, python2pressure, pressure2python);

    /* Visual appearance */
    registerProperty("AmbientColor", color_str2any, color_any2str, color_python2any, color_any2python);
    registerProperty("DiffuseColor", color_str2any, color_any2str, color_python2any, color_any2python);
    registerProperty("SpecularColor", color_str2any, color_any2str, color_python2any, color_any2python);
    registerProperty("EmissiveColor", color_str2any, color_any2str, color_python2any, color_any2python);
    registerProperty("Shininess", float2any, any2float, python2float, float2python);
    registerProperty("Transparency", float2any, any2float, python2float, float2python);

    registerProperty("VertexShader", string2any, any2string, python2string, string2python);
    registerProperty("FragmentShader", string2any, any2string, python2string, string2python);

    /* Optical properties */

    /* FEM */

    /* BIM */
    registerProperty("StandardFormat", string2any, any2string, python2string, string2python);
    registerProperty("StandardCode", string2any, any2string, python2string, string2python);
    registerProperty("FireStandard", string2any, any2string, python2string, string2python);
    registerProperty("FireClass", string2any, any2string, python2string, string2python);
    registerProperty("ThermalConductivity", float2any, any2float, python2float, float2python);
    registerProperty("SoundTransmission", string2any, any2string, python2string, string2python);
    registerProperty("Finish", string2any, any2string, python2string, string2python);
    registerProperty("Color", string2any, any2string, python2string, string2python);
    registerProperty("UnitsArea", string2any, any2string, python2string, string2python);

    PythonObject = Py::asObject(new Py::MaterialDatabasePy(this));
}

void MaterialDatabase::addMaterialSource(std::shared_ptr<MaterialSource> source)
{
    sources.push_back(source);

    source->setDatabase(this);
}

int MaterialDatabase::registerProperty(const char *propName,
                                       fromStringToAnyFunction fromStringToAny,
                                       toStringFromAnyFunction toStringFromAny,
                                       fromPythonToAnyFunction fromPythonToAny,
                                       toPythonFromAnyFunction toPythonFromAny)
{
    int id = getPropertyId(propName);

    if (id == -1) {
        id = propertyMap.size();
        propertyMap[propName] = PropertyInfo{id, fromStringToAny, toStringFromAny, fromPythonToAny, toPythonFromAny};
        propertyIdMap[id] = propName;
    }

    return id;
}

int MaterialDatabase::registerUnknownProperty(const char *propName)
{
    int id = getPropertyId(propName);

    if (id == -1) {
        id = propertyMap.size();
        propertyMap[propName] = PropertyInfo{id, string2any, any2string, python2string, string2python};
        propertyIdMap[id] = propName;
    }

    return id;
}

int MaterialDatabase::getPropertyId(const char *propName) const
{
    auto it = propertyMap.find(propName);

    if (it == propertyMap.end())
        return -1;
    else
        return it->second.id;
}

const char *MaterialDatabase::getPropertyName(int id) const
{
    auto it = propertyIdMap.find(id);

    if (it != propertyIdMap.end())
        return it->second.c_str();
    else
        return "<unregistered>";
}

void MaterialDatabase::getMaterials(std::vector<Material*> & materials) const
{
    for (auto it = sources.rbegin(); it != sources.rend(); ++it)
        (*it)->getMaterials(materials);

    if (parent)
        parent->getMaterials(materials);
}

MaterialSource *MaterialDatabase::getMaterialSource(const char *name)
{
    for (auto it = sources.begin(); it != sources.end(); ++it)
        if ((*it)->getName() == name)
            return (*it).get();

    if (parent)
        return parent->getMaterialSource(name);

    return 0;
}

size_t MaterialDatabase::getNumProperties() const
{
    return propertyMap.size();
}

Material *MaterialDatabase::getMaterial(const char *materialName)
{
    for (auto it = sources.rbegin(); it != sources.rend(); ++it) {
        Material * mat = (*it)->getMaterial(materialName);

        if (mat != 0)
            return mat;
    }

    if (parent)
        return parent->getMaterial(materialName);

    return 0;
}

PyObject *MaterialDatabase::getPyObject()
{
    return Py::new_reference_to(PythonObject);
}

void MaterialDatabase::commit()
{
    for (auto it = sources.begin(); it != sources.end(); ++it)
        (*it)->commit();
}

boost::any MaterialDatabase::fromString(const std::string &key, const std::string &value) const
{
    auto it = propertyMap.find(key);

    if (it == propertyMap.end())
        throw Base::Exception(str(boost::format("Invalid property: %1%") % key));

    return it->second.fromStringToAny(value);
}

std::string MaterialDatabase::toString(const std::string &key, const boost::any &value) const
{
    auto it = propertyMap.find(key);

    if (it == propertyMap.end())
        throw Base::Exception(str(boost::format("Invalid property: %1%") % key));

    if (value.empty())
        throw Base::Exception(str(boost::format("Property %1% is empty.") % key));

    return it->second.toStringFromAny(value);
}

PyObject *MaterialDatabase::toPyObject(const char *propName, const boost::any &value) const
{
    auto it = propertyMap.find(propName);

    if (it == propertyMap.end())
        throw Base::Exception(str(boost::format("Invalid property: %1%") % propName));

    if (value.empty())
        throw Base::Exception(str(boost::format("Property %1% not found.") % propName));

    return it->second.toPythonFromAny(value);
}

boost::any MaterialDatabase::fromPyObject(const char *propName, const PyObject *value) const
{
    auto it = propertyMap.find(propName);

    if (it == propertyMap.end())
        throw Base::Exception(str(boost::format("Invalid property: %1%") % propName));

    return it->second.fromPythonToAny(value);
}

Py::MaterialDatabasePy::MaterialDatabasePy(MaterialDatabase *_twin)
    : twin(_twin)
{

}

Py::Object Py::MaterialDatabasePy::addMaterialSource(const Py::Tuple &args)
{
    if (Py::MaterialSourcePy::check(args[0])) {
        const Py::MaterialSourcePy * source = static_cast<const Py::MaterialSourcePy*>(args[0].ptr());

        twin->addMaterialSource(std::shared_ptr<App::MaterialSource>(source->twin));

        return Py::None();
    }
    else
        throw Py::TypeError("Invalid type: MaterialSource required.");
}

Py::Object Py::MaterialDatabasePy::getMaterialSource(const Py::Tuple &args)
{
    std::string name = Py::String(args[0]);

    MaterialSource * source = twin->getMaterialSource(name.c_str());

    return Py::asObject(source->getPyObject());
}

Py::Object Py::MaterialDatabasePy::getMaterials()
{
    std::vector<Material*> materials;

    twin->getMaterials(materials);

    Py::List list;

    for (auto it : materials)
        list.append(Py::asObject(it->getPyObject()));

    return list;
}

Py::Object Py::MaterialDatabasePy::getMaterial(const Py::Tuple &args)
{
    std::string name = Py::String(args[0]);

    Material * mat = twin->getMaterial(name.c_str());

    if (mat)
        return Py::asObject(mat->getPyObject());
    else
        return Py::None();
}

Py::Object Py::MaterialDatabasePy::getattr(const char *name)
{
    return getattr_methods( name );
}

void Py::MaterialDatabasePy::init_type()
{
    behaviors().name( "MaterialDatabase" );
    behaviors().doc( "Documentation for MaterialDatabase class" );
    behaviors().supportGetattr();

    add_varargs_method("addMaterialSource", &Py::MaterialDatabasePy::addMaterialSource);
    add_varargs_method("getMaterialSource", &Py::MaterialDatabasePy::getMaterialSource);
    add_noargs_method("getMaterials", &Py::MaterialDatabasePy::getMaterials);
    add_varargs_method("getMaterial", &Py::MaterialDatabasePy::getMaterial);

    // Call to make the type ready for use
    behaviors().readyType();
}

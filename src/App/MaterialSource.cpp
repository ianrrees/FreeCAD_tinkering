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

#include <memory>
#include <boost/any.hpp>

#include "MaterialSource.h"
#include "MaterialDatabase.h"
#include "Material.h"
#include "MaterialPy.h"

using namespace App;

TYPESYSTEM_SOURCE(App::MaterialSource, Base::BaseClass);

MaterialSource::MaterialSource(const char * _name)
    : name(_name)
    , database(0)
{
    PythonObject = Py::Object(new Py::MaterialSourcePy(this), true);
}

int App::MaterialSource::getPropertyId(const char *propertyName) const
{
    if (!database)
       throw Base::Exception("getPropertyId: Source not added to a database.");

    return database->getPropertyId(propertyName);
}

const char *MaterialSource::getPropertyName(int id) const
{
    if (!database)
       throw Base::Exception("getPropertyName: Source not added to a database.");

    return database->getPropertyName(id);
}

size_t MaterialSource::getNumProperties() const
{
    if (!database)
       throw Base::Exception("getNumProperties: Source not added to a database.");

    return database->getNumProperties();
}

int MaterialSource::registerUnknownProperty(const char *propName)
{
    return getDatabase()->registerUnknownProperty(propName);
}

Material *MaterialSource::createMaterial(const std::vector<boost::any> &properties)
{
    if (!database)
       throw Base::Exception("createMaterial: Source not added to a database.");

    if (isReadOnly())
        throw Base::Exception("Unable to create material; material source is read-only.");

    std::shared_ptr<Material> mat = std::make_shared<Material>(this, properties);

    material[boost::any_cast<std::string>(mat->getProperty("Name"))] = mat;

    return mat.get();
}

Material *MaterialSource::createMaterial(const char *materialName)
{
    if (!database)
       throw Base::Exception("createMaterial: Source not added to a database.");

    if (isReadOnly())
        throw Base::Exception("Unable to create material; material source is read-only.");

    std::vector<boost::any> properties;
    std::shared_ptr<Material> mat = std::make_shared<Material>(this, properties);

    mat->setProperty("Name", std::string(materialName));

    material[materialName] = mat;
    return mat.get();
}

Material *MaterialSource::getMaterial(const char *materialName) const
{
    if (!database)
       throw Base::Exception("getMaterial: Source not added to a database.");

    auto it = material.find(materialName);

    if (it == material.end())
        return 0;
    else
        return it->second.get();
}

Material *MaterialSource::getOrCreateMaterial(const char *materialName)
{
    Material * mat = getMaterial(materialName);

    if (!mat)
        mat = createMaterial(materialName);

    return mat;
}

void MaterialSource::getMaterials(std::vector<Material *> &materials) const
{
    if (!database)
       throw Base::Exception("getMaterials: Source not added to a database.");

    for (auto it = material.begin(); it != material.end(); ++it)
        materials.push_back(it->second.get());
}

void MaterialSource::removeMaterial(const Material *mat)
{
    if (!isReadOnly())
        material.erase(mat->getName());
}

PyObject *MaterialSource::toPyObject(const char *propName, const boost::any &value) const
{
    if (!database)
       throw Base::Exception("toPyObject: Source not added to a database.");

    return database->toPyObject(propName, value);
}

boost::any MaterialSource::fromPyObject(const char *propName, const PyObject *value) const
{
    if (!database)
       throw Base::Exception("fromPyObject: Source not added to a database.");

    return database->fromPyObject(propName, value);
}

std::string MaterialSource::toString(int id, const boost::any &value) const
{
    if (!database)
       throw Base::Exception("toString: Source not added to a database.");

    return getDatabase()->toString(getPropertyName(id), value);
}

boost::any MaterialSource::fromString(std::string &key, const std::string &value) const
{
    if (!database)
       throw Base::Exception("fromString: Source not added to a database.");

    return getDatabase()->fromString(key, value);
}

PyObject * MaterialSource::getPyObject()
{
    return Py::new_reference_to(PythonObject);
}

bool MaterialSource::isReadOnly() const
{
    return true;
}

void App::MaterialSource::setDatabase(MaterialDatabase *_database)
{
    this->database = _database;
}

Py::MaterialSourcePy::MaterialSourcePy(MaterialSource *_twin)
    : twin(_twin)
{
}

Py::Object Py::MaterialSourcePy::createMaterial(const Py::Tuple &args)
{
    const std::string name = Py::String(args[0]);

    twin->createMaterial(name.c_str());

    return Py::None();
}

Py::Object Py::MaterialSourcePy::getMaterial(const Py::Tuple &args)
{
    const std::string name = Py::String(args[0]);
    Material * mat = twin->getMaterial(name.c_str());

    if (mat)
        return Py::asObject(mat->getPyObject());
    else
        return Py::None();
}

Py::Object Py::MaterialSourcePy::getOrCreateMaterial(const Py::Tuple &args)
{
    const std::string name = Py::String(args[0]);
    Material * mat = twin->getOrCreateMaterial(name.c_str());

    return Py::asObject(mat->getPyObject());
}

Py::Object Py::MaterialSourcePy::getMaterials()
{
    std::vector<App::Material*> materials;

    twin->getMaterials(materials);

    return Py::None();
}

Py::Object Py::MaterialSourcePy::removeMaterial(const Py::Tuple &args)
{
    throw Py::NotImplementedError("Not implemented");
    return Py::None();
}

void Py::MaterialSourcePy::init_type()
{
    behaviors().name( "MaterialSource" );
    behaviors().doc( "Documentation for MaterialSource class" );
    behaviors().supportGetattr();

    add_varargs_method("createMaterial", &Py::MaterialSourcePy::createMaterial);
    add_varargs_method("getMaterial", &Py::MaterialSourcePy::getMaterial);
    add_noargs_method("getMaterials", &Py::MaterialSourcePy::getMaterials);
    add_varargs_method("getOrCreateMaterial", &Py::MaterialSourcePy::getOrCreateMaterial);
    add_varargs_method("removeMaterial", &Py::MaterialSourcePy::removeMaterial);

    // Call to make the type ready for use
    behaviors().readyType();
}

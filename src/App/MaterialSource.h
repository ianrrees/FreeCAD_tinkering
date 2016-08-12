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

#ifndef APP_MATERIALSOURCE_H
#define APP_MATERIALSOURCE_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

#include <Base/BaseClass.h>

#include <CXX/Extensions.hxx>

namespace App {

class MaterialDatabase;
class Material;

class AppExport MaterialSource :  public Base::BaseClass, private boost::noncopyable
{
    TYPESYSTEM_HEADER();
public:

    MaterialSource(const char * name = 0);

    int getPropertyId(const char * propertyName) const;

    const char * getPropertyName(int id) const;

    const std::string & getName() const { return name; }

    size_t getNumProperties() const;

    int registerUnknownProperty(const char * propName);

    virtual Material * createMaterial(const char * materialName);

    virtual Material * createMaterial(const std::vector<boost::any> &properties);

    virtual Material * getMaterial(const char * materialName) const;

    virtual Material * getOrCreateMaterial(const char * materialName);

    virtual void getMaterials(std::vector<App::Material *> &materials) const;

    virtual void removeMaterial(const Material * material);

    virtual PyObject *getPyObject(void);

    virtual bool isReadOnly() const;

    virtual bool canUndo() const { return false; };

    virtual void commit() { }

    virtual size_t getNumMaterials() const { return material.size(); }

    std::string toString(int id, const boost::any & value) const;

    boost::any fromString(std::string & key, const std::string & value) const;

protected:
    friend class Material;

    App::MaterialDatabase * getDatabase() const { return database; }

    PyObject * toPyObject(const char * propName, const boost::any & value) const;

    boost::any fromPyObject(const char * propName, const PyObject * value) const;

    friend class MaterialDatabase;

    virtual void setDatabase(App::MaterialDatabase * _database);

    Py::Object PythonObject;
    std::string name;
    App::MaterialDatabase * database;
    mutable std::map<std::string, std::shared_ptr<Material> > material;
};

}

namespace Py {

class MaterialDatabasePy;

class MaterialSourcePy : public Py::PythonExtension<MaterialSourcePy>
{

public:
    MaterialSourcePy(App::MaterialSource * _twin);

    Py::Object createMaterial(const Py::Tuple &args);

    Py::Object getMaterial(const Py::Tuple &args);

    Py::Object getOrCreateMaterial(const Py::Tuple &args);

    Py::Object getMaterials();

    Py::Object removeMaterial(const Py::Tuple &args);

    static void init_type(void);

private:
    friend class Py::MaterialDatabasePy;

    App::MaterialSource * twin;
};

}

#endif // MATERIALSOURCE_H

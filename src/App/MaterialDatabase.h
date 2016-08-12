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

#ifndef APP_MATERIALDATABASE_H
#define APP_MATERIALDATABASE_H

#include <vector>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <map>

#include <Base/BaseClass.h>
#include "Material.h"
#include "MaterialSource.h"

#include <CXX/Extensions.hxx>

namespace App {

class MaterialSource;


class AppExport MaterialDatabase : public Base::BaseClass, private boost::noncopyable
{
    TYPESYSTEM_HEADER();

public:
    typedef boost::function<boost::any(const std::string&)> fromStringToAnyFunction;
    typedef boost::function<std::string(const boost::any&)> toStringFromAnyFunction;
    typedef boost::function<PyObject*(const boost::any&)> toPythonFromAnyFunction;
    typedef boost::function<boost::any(const PyObject*)> fromPythonToAnyFunction;

    struct PropertyInfo {
        int id;
        fromStringToAnyFunction fromStringToAny;
        toStringFromAnyFunction toStringFromAny;
        fromPythonToAnyFunction fromPythonToAny;
        toPythonFromAnyFunction toPythonFromAny;
    };

    MaterialDatabase(MaterialDatabase * _parent = 0);

    /** Add a material source */
    void addMaterialSource(std::shared_ptr<MaterialSource> source);

    /** Register a property */
    int registerProperty(const char * propName,
                                   fromStringToAnyFunction fromStringToAny,
                                   toStringFromAnyFunction toStringFromAny,
                                   fromPythonToAnyFunction fromPythonToAny,
                                   toPythonFromAnyFunction toPythonFromAny);

    int registerUnknownProperty(const char * propName);

    /** Get an identifier to a material property. Note: IDs are transient and MUST NOT be stored externally for later use */
    int getPropertyId(const char * propName) const;

    const char * getPropertyName(int id) const;

    /** Get a list of materials, aggregated from all sources */
    void getMaterials(std::vector<Material *> &materials) const;

    MaterialSource * getMaterialSource(const char * name);

    Material *getMaterial(const char * materialName);

    virtual PyObject *getPyObject(void);

    void commit();

    /* Converters */

    boost::any fromString(const std::string &key, const std::string & value) const;

    std::string toString(const std::string &key, const boost::any & value) const;

    PyObject * toPyObject(const char * propName, const boost::any & value) const;

    boost::any fromPyObject(const char * propName, const PyObject * value) const;

    /* Static converters */
    static std::string color_any2str(const boost::any & data);
    static boost::any color_str2any(const std::string & data);
    static PyObject * color_any2python(const boost::any & data);
    static boost::any color_python2any(const PyObject * data);

    static std::string any2float(const boost::any & data);
    static boost::any float2any(const std::string & data);
    static PyObject * float2python(const boost::any & data);
    static boost::any python2float(const PyObject * data);

    static std::string any2string(const boost::any &data);
    static boost::any string2any(const std::string &data);
    static PyObject *string2python(const boost::any &data);
    static boost::any python2string(const PyObject *data);

    static std::string any2density(const boost::any &data);
    static boost::any density2any(const std::string &data);
    static PyObject *density2python(const boost::any &data);
    static boost::any python2density(const PyObject *data);

    static std::string any2pressure(const boost::any &data);
    static boost::any pressure2any(const std::string &data);
    static PyObject *pressure2python(const boost::any &data);
    static boost::any python2pressure(const PyObject *data);

private:

    friend class MaterialSource;

    /** Get number of registered properties */
    size_t getNumProperties() const;

    Py::Object PythonObject;
    MaterialDatabase * parent;
    std::vector<std::shared_ptr<MaterialSource> > sources;
    static std::map<std::string, PropertyInfo> propertyMap;
    static std::map<int, std::string> propertyIdMap;
};

}

namespace Py {

class MaterialDatabasePy : public Py::PythonExtension<MaterialDatabasePy>
{

public:
    MaterialDatabasePy(App::MaterialDatabase * _twin);

    Py::Object addMaterialSource(const Py::Tuple &args);

    Py::Object getMaterialSource(const Py::Tuple &args);

    Py::Object getMaterials();

    Py::Object getMaterial(const Py::Tuple &args);

    Py::Object getattr( const char *name );

    static void init_type(void);

private:

    App::MaterialDatabase * twin;
};

}

#endif // MATERIALDATABASE_H

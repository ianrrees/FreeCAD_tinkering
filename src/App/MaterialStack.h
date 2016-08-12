/***************************************************************************
 *   Copyright (c) Eivind Kvedalen         (eivind@kvedalen.name) 2016     *
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

#ifndef APP_MATERIALSTACK_H
#define APP_MATERIALSTACK_H

#include <Base/Persistence.h>
#include <boost/any.hpp>
#include <CXX/Extensions.hxx>

namespace Base {
class Writer;
class Reader;
}

namespace App {
class Material;
class MaterialDatabase;
class PropertyPartMaterial;

class MaterialStack : public Base::Persistence {

public:

    MaterialStack(PropertyPartMaterial *_owner);

    boost::any getProperty(unsigned long index, int id) const;

    void insert(const std::string & name, int index = -1);

    void clear() {
        materials.clear();
    }

    void remove(int index);

    size_t size() const { return materials.size(); }

    virtual PyObject *getPyObject(void);

    const std::string & get(int index) const;

    Material *getMaterial(int index) const;

    void Save(Base::Writer &writer) const;

    void Restore(Base::XMLReader &reader);

    unsigned int getMemSize (void) const { return 0; }

private:

    MaterialDatabase * getDatabase() const;

    PyObject * PythonObject;
    PropertyPartMaterial * owner;
    std::vector<std::string> materials;
};

}

namespace Py {

class MaterialStackPy : public Py::PythonExtension<Py::MaterialStackPy> {

public:
    MaterialStackPy(App::MaterialStack * _twin);

    static void init_type(void);

    Py::Object getattr( const char *name );

    virtual Py::Object sequence_item(Py_ssize_t i);

    Py::Object insert(const Py::Tuple& args);

    Py::Object clear();

    Py::Object remove(const Py::Tuple& args);

    Py::Object getLayer(const Py::Tuple& args);

private:
    App::MaterialStack * twin;
};

}

#endif // MATERIALSTACK_H

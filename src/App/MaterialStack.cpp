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

#include "PreCompiled.h"

#include <Base/Exception.h>
#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Tools.h>
#include <boost/format.hpp>

#include "MaterialStack.h"
#include "Material.h"
#include "MaterialPy.h"
#include "MaterialDatabase.h"
#include "PropertyPartMaterial.h"

using namespace App;

MaterialStack::MaterialStack(PropertyPartMaterial * _owner)
    : Base::Persistence()
    , PythonObject(0)
    , owner(_owner)
{

}

boost::any MaterialStack::getProperty(unsigned long index, int id) const {
    if (index < 0 || index >= materials.size())
        throw Base::Exception(str(boost::format("Index %1% out of range") % index));

    Material * mat = getDatabase()->getMaterial(materials[index].c_str());

    return mat->getProperty(id);
}

void MaterialStack::insert(const std::string &name, int index)
{
    if (index == -1)
        materials.push_back(name);
    else if (index >= 0 && (size_t)index <= materials.size())
        materials.insert(materials.begin() + index, name);
    else
        throw Base::Exception(str(boost::format("Index %1% out of range") % index));
}

void MaterialStack::remove(int index)
{
    if (index >= 0 && (size_t)index < materials.size())
        materials.erase(materials.begin() + index);
    else
        throw Base::Exception(str(boost::format("Index %1% out of range") % index));
}

PyObject *MaterialStack::getPyObject() {
    return PythonObject;
}

const std::string &MaterialStack::get(int index) const
{
    if (index < 0 || (size_t)index >= materials.size())
        throw Base::Exception(str(boost::format("Index %1% out of range") % index));

    return materials[index];
}

Material *MaterialStack::getMaterial(int index) const
{
    if (index < 0 || (size_t)index >= materials.size())
        throw Base::Exception(str(boost::format("Index %1% out of range") % index));

    return getDatabase()->getMaterial(materials[index].c_str());
}

void MaterialStack::Save(Base::Writer & writer) const
{
    auto vit = materials.begin();
    while (vit != materials.end()) {
        writer.Stream() << writer.ind() << "<Material name=\"" <<  Base::Tools::encodeAttribute(*vit) <<"\"/>";
        ++vit;
    }
}

void MaterialStack::Restore(Base::XMLReader &reader)
{
    int faceCount = reader.getAttributeAsInteger("count");

    for (int j = 0; j < faceCount; ++j) {
        reader.readElement("Material");
        std::string name = reader.getAttribute("name");

        insert(name);
    }
}

MaterialDatabase *MaterialStack::getDatabase() const
{
    return owner->getDatabase();
}

Py::MaterialStackPy::MaterialStackPy(MaterialStack *_twin)
    : twin(_twin)
{

}

void Py::MaterialStackPy::init_type() {
    behaviors().name( "MaterialStack" );
    behaviors().doc( "Documentation for MaterialStack class" );
    behaviors().supportGetattr();
    behaviors().supportSequenceType();

    add_varargs_method("insert", &Py::MaterialStackPy::insert);
    add_noargs_method("clear", &Py::MaterialStackPy::clear);
    add_varargs_method("remove", &Py::MaterialStackPy::remove);
    add_varargs_method("getLayer", &Py::MaterialStackPy::getLayer);

    // Call to make the type ready for use
    behaviors().readyType();
}

Py::Object Py::MaterialStackPy::getattr(const char *name) {
    return getattr_methods( name );
}

Py::Object Py::MaterialStackPy::sequence_item(Py_ssize_t i)
{
    return Py::String(twin->get(i).c_str());
}

Py::Object Py::MaterialStackPy::insert(const Py::Tuple &args)
{
    String materialName(args[0]);
    Py::Int index(args.size() == 2 ? Py::Int(args[1]) : Py::Int(-1));

    if (index < -1 || (size_t)index >= twin->size())
        throw Py::IndexError(boost::str(boost::format("Index %1% out of range") % index));

    twin->insert(materialName, index);
    return Py::None();
}

Py::Object Py::MaterialStackPy::clear()
{
    twin->clear();
    return Py::None();
}

Py::Object Py::MaterialStackPy::remove(const Py::Tuple &args)
{
    Py::Int index(args[0]);

    if (index < 0 || (size_t)index >= twin->size())
        throw Py::IndexError(boost::str(boost::format("Index %1% out of range") % index));

    twin->remove(index);
    return Py::None();
}

Py::Object Py::MaterialStackPy::getLayer(const Py::Tuple &args)
{
    Py::Int index(args[0]);

    if (index < 0 || (size_t)index >= twin->size())
        throw Py::IndexError(boost::str(boost::format("Index %1% out of range") % index));

    return Py::asObject(twin->getMaterial(index)->getPyObject());
}

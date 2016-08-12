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

#include "MaterialComposition.h"
#include "Material.h"
#include "MaterialDatabase.h"
#include "PropertyPartMaterial.h"
#include "MaterialStack.h"

namespace Py {

class SurfaceMaterialArray : public PythonExtension<SurfaceMaterialArray> {

public:
    SurfaceMaterialArray(App::PropertyPartMaterial * _owner, int _solid)
        : owner(_owner)
        , solid(_solid)
    {

    }

    static void init_type() {
        behaviors().name("SurfaceMaterialArray");
        behaviors().doc("SurfaceMaterialArray class, to support arrays of MaterialStack objects");
        behaviors().supportSequenceType();
        behaviors().supportGetattr();
        behaviors().readyType();
    }

    virtual Py::Object sequence_item(Py_ssize_t i) {
        if (!owner->getSurfaceMaterials(solid, i))
            throw IndexError(boost::str(boost::format("Index %1% out of range") % i));

        return Py::asObject(new MaterialStackPy(owner->getSurfaceMaterials(solid, i)));
    }

private:
    App::PropertyPartMaterial * owner;
    int solid;
};

}

using namespace App;

MaterialComposition::MaterialComposition(PropertyPartMaterial *_owner, int _solid)
    : Base::Persistence()
    , PythonObject(0)
    , owner(_owner)
{

}

boost::any MaterialComposition::getProperty(int id) const
{
    for (auto it = materials.rbegin(); it != materials.rend(); ++it) {
        Material * mat = getDatabase()->getMaterial((*it).c_str());
        if (!mat)
            throw Base::Exception(str(boost::format("Unknown material: %1%") % *it));
        boost::any value = mat->getProperty(id);
        if (!value.empty())
            return value;
    }

    return boost::any();
}

PyObject * MaterialComposition::getPyProperty(const char * name) const
{
    int id = getDatabase()->getPropertyId(name);

    if (id == -1)
        return 0;

    boost::any value = getProperty(id);

    if (value.empty())
        return 0;
    else
        return getDatabase()->toPyObject(name, value);
}

void MaterialComposition::insert(const std::string &name, int index) {
    if (index == -1)
        materials.push_back(name);
    else if (index >= 0 && (size_t)index <= materials.size())
        materials.insert(materials.begin() + index, name);
    else
        throw Base::Exception(str(boost::format("Inedex %1% out of range") % index));
}

void MaterialComposition::remove(int index) {
    if (index >= 0 && (size_t)index < materials.size())
        materials.erase(materials.begin() + index);
    else
        throw Base::Exception(str(boost::format("Inedex %1% out of range") % index));
}

const std::string &MaterialComposition::get(int index) const
{
    if (index < 0 || (size_t)index >= materials.size())
        throw Base::Exception(str(boost::format("Inedex %1% out of range") % index));

    return materials[index];
}

void MaterialComposition::Save(Base::Writer &writer) const
{
    writer.Stream() << writer.ind() << "<Solid count=\"" << materials.size() << "\">";
    writer.incInd();

    for (auto vit = materials.begin(); vit != materials.end(); ++vit)
        writer.Stream() << writer.ind() << "<Material name=\"" <<  Base::Tools::encodeAttribute(*vit) <<"\"/>";

    writer.decInd();
    writer.Stream() << writer.ind() << "</Solid>";
}

void MaterialComposition::Restore(Base::XMLReader &reader)
{
    reader.readElement("Solid");
    int matCount = reader.getAttributeAsInteger("count");

    for (int j = 0; j < matCount; ++j) {
        reader.readElement("Material");
        std::string name = reader.getAttribute("name");

        insert(name);
    }
    reader.readEndElement("Solid");
}

MaterialDatabase *MaterialComposition::getDatabase() const
{
    return owner->getDatabase();
}


Py::MaterialCompositionPy::MaterialCompositionPy(App::PropertyPartMaterial *_owner, int _solid)
    : owner(_owner)
    , solid(_solid)
{

}

void Py::MaterialCompositionPy::init_type() {
    behaviors().name( "MaterialComposition" );
    behaviors().doc( "Documentation for MaterialComposition class" );
    behaviors().supportGetattr();

    add_varargs_method("setSurfaceMaterial", &Py::MaterialCompositionPy::setSurfaceMaterial);
    add_varargs_method("addSurfaceMaterial", &Py::MaterialCompositionPy::addSurfaceMaterial);
    add_varargs_method("removeSurfaceMaterial", &Py::MaterialCompositionPy::removeSurfaceMaterial);
    add_varargs_method("getSurfaceMaterialCount", &Py::MaterialCompositionPy::getSurfaceMaterialCount);

    // Call to make the type ready for use
    behaviors().readyType();

    Py::SurfaceMaterialArray::init_type();
}

Py::Object Py::MaterialCompositionPy::getattr(const char *name)
{
    if (strcmp(name, "Surface") == 0)
        return Py::asObject(new Py::SurfaceMaterialArray(owner, solid));
    else {
        const App::MaterialComposition * mc = owner->getSolidMaterials(solid);

        if (mc) {
            try {
                PyObject * value = mc->getPyProperty(name);

                if (value)
                    return Py::asObject(value);
            }
            catch (Base::Exception & e) {
                throw Py::Exception(boost::str(boost::format("Unable to resolve property %1% (missing material?)") % name));
            }
        }

        return getattr_methods( name );
    }
}

Py::Object Py::MaterialCompositionPy::setSurfaceMaterial(const Tuple &args)
{
    Int face(args[0]);
    String materialName(args[1]);

    owner->setSurfaceMaterial(solid, face, materialName);
    return Py::None();
}

Py::Object Py::MaterialCompositionPy::addSurfaceMaterial(const Tuple &args)
{
    Int face(args[0]);
    String materialName(args[1]);

    owner->addSurfaceMaterial(solid, face, materialName);
    return Py::None();
}

Py::Object Py::MaterialCompositionPy::removeSurfaceMaterial(const Tuple &args)
{
    Int face(args[0]);
    Int index(args[1]);

    owner->removeSurfaceMaterial(solid, face, index);
    return Py::None();
}

Py::Object Py::MaterialCompositionPy::getSurfaceMaterialCount(const Tuple &args)
{
    Int face(args[0]);

    owner->getSurfaceMaterialCount(solid, face);
    return Py::None();
}

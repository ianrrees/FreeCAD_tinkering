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

#ifndef _PreComp_
#endif

#include <Base/Exception.h>
#include <Base/Writer.h>
#include <Base/Reader.h>
#include <Base/Tools.h>
#include <boost/format.hpp>
#include "MaterialStack.h"
#include "MaterialPy.h"
#include "MaterialComposition.h"
#include "MaterialDatabase.h"
#include "DocumentObject.h"
#include "Document.h"
#include "PropertyPartMaterial.h"

using namespace App;

TYPESYSTEM_SOURCE(App::PropertyPartMaterial , App::Property);

PropertyPartMaterial::PropertyPartMaterial()
    : Property()
{
    
}

void PropertyPartMaterial::setValue(const PropertyPartMaterial & value)
{
    Paste(value);
}

void PropertyPartMaterial::setSolidMaterial(unsigned long solid, const std::string &name)
{
    if (solidMaterials.size() != 1 || solidMaterials[solid]->get(0) != name) {
        aboutToSetValue();
        if (solidMaterials.size() <= solid)
            solidMaterials.resize(solid + 1);

        if (!solidMaterials[solid])
            solidMaterials[solid] = std::make_shared<MaterialComposition>(this, solid);

        solidMaterials[solid]->clear();
        solidMaterials[solid]->insert(name);
        hasSetValue();
    }
}

void PropertyPartMaterial::addSolidMaterial(unsigned long solid, const std::string &name, int index)
{
    if (solidMaterials.size() <= solid) {
        aboutToSetValue();
        if (solidMaterials.size() <= solid)
            solidMaterials.resize(solid + 1);

        if (!solidMaterials[solid])
            solidMaterials[solid] = std::make_shared<MaterialComposition>(this, solid);

        solidMaterials[solid]->insert(name, index);
        hasSetValue();
    }
}

void PropertyPartMaterial::removeSolidMaterial(unsigned long solid, int index)
{
    aboutToSetValue();
    if (solidMaterials.size() <= solid)
        solidMaterials.resize(solid + 1);
    solidMaterials[solid]->remove(index);
    hasSetValue();
}

size_t PropertyPartMaterial::getSolidMaterialCount(unsigned long solid) const
{
    if (solidMaterials.size() <= solid)
        return 0;
    return solidMaterials[solid]->size();
}

boost::any PropertyPartMaterial::getSolidProperty(unsigned long solid, const char *propName) const
{
    return getSolidProperty(solid, getDatabase()->getPropertyId(propName));
}

boost::any PropertyPartMaterial::getSolidProperty(unsigned long solid, int id) const
{
    if (solidMaterials.size() >= solid && solidMaterials[solid])
        return solidMaterials[solid]->getProperty(id);
    else
        return boost::any();
}

const MaterialComposition *PropertyPartMaterial::getSolidMaterials(unsigned long solid) const
{
    if (solid < solidMaterials.size())
        return solidMaterials[solid].get();
    else
        return 0;
}

void PropertyPartMaterial::setSurfaceMaterial(unsigned long solid, unsigned long face, const std::string &name)
{
    std::pair<unsigned long, unsigned long> p(solid, face);

    aboutToSetValue();
    if (!surfaceMaterials[p])
        surfaceMaterials[p] = std::make_shared<MaterialStack>(this);
    surfaceMaterials[p]->clear();
    surfaceMaterials[p]->insert(name);
    hasSetValue();
}

void PropertyPartMaterial::addSurfaceMaterial(unsigned long solid, unsigned long face, const std::string &name)
{
    std::pair<unsigned long, unsigned long> p(solid, face);

    aboutToSetValue();
    if (!surfaceMaterials[p])
        surfaceMaterials[p] = std::make_shared<MaterialStack>(this);
    surfaceMaterials[p]->insert(name);
    hasSetValue();
}

void PropertyPartMaterial::removeSurfaceMaterial(unsigned long solid, unsigned long face, unsigned long index)
{
    std::pair<unsigned long, unsigned long> p(solid, face);

    aboutToSetValue();
    if (!surfaceMaterials[p])
        surfaceMaterials[p] = std::make_shared<MaterialStack>(this);
    surfaceMaterials[p]->remove(index);
    hasSetValue();
}

size_t PropertyPartMaterial::getSurfaceMaterialCount(unsigned long solid, unsigned long face) const
{
    std::pair<unsigned long, unsigned long> p(solid, face);
    auto it = surfaceMaterials.find(p);

    if (it == surfaceMaterials.end())
        return 0;
    else
        return it->second->size();
}

MaterialStack * PropertyPartMaterial::getSurfaceMaterials(unsigned long solid, unsigned long face) const
{
    std::pair<unsigned long, unsigned long> p(solid, face);

    auto it = surfaceMaterials.find(p);

    if (it == surfaceMaterials.end())
        return 0;
    else
        return it->second.get();
}

boost::any PropertyPartMaterial::getSurfaceProperty(unsigned long solid, unsigned long face, unsigned long index, const char *propName) const
{
    return getSurfaceProperty(solid, face, index, getDatabase()->getPropertyId(propName));
}

boost::any PropertyPartMaterial::getSurfaceProperty(unsigned long solid, unsigned long face, unsigned long index, int id) const
{
    std::pair<unsigned long, unsigned long> p(solid, face);
    auto it = surfaceMaterials.find(p);

    if (it == surfaceMaterials.end())
        return boost::any();
    else
        return it->second->getProperty(index, id);
}

Property *PropertyPartMaterial::Copy() const
{
    auto p = new PropertyPartMaterial();
    
    p->surfaceMaterials = surfaceMaterials;
    p->solidMaterials = solidMaterials;
    
    return p;
}

void PropertyPartMaterial::Paste(const Property &from)
{
    auto p = static_cast<const PropertyPartMaterial*>(&from);
    
    solidMaterials = p->solidMaterials;
    surfaceMaterials = p->surfaceMaterials;
}

PyObject *PropertyPartMaterial::getPyObject()
{
    if (PythonObject.is(Py::_None())) {
        // ref counter is set to 1
        PythonObject = Py::Object(new Py::PropertyPartMaterial(this), true);
    }
    return Py::new_reference_to(PythonObject);
}

void PropertyPartMaterial::setPyObject(PyObject * value)
{
    throw Base::Exception("Assignment not supported.");
}

void PropertyPartMaterial::Save(Base::Writer &writer) const
{
    writer.Stream() << writer.ind() << "<PropertyPartMaterial solidCount=\"" << solidMaterials.size() << "\" surfaceCount=\"" << surfaceMaterials.size() << "\">" << std::endl;
    writer.incInd();

    for (auto it = solidMaterials.begin(); it != solidMaterials.end(); ++it)
        (*it)->Save(writer);

    for (auto it = surfaceMaterials.begin(); it != surfaceMaterials.end(); ++it) {
        const auto & v = it->second;
        auto p = it->first;

        writer.Stream() << writer.ind() << "<Face solid=\"" << p.first << "\" face=\"" << p.second <<"\" count=\"" << v->size() << "\">";
        writer.incInd();

        it->second->Save(writer);

        writer.decInd();
        writer.Stream() << "</Face>" << std::endl;
    }
    writer.decInd();
    writer.Stream() << writer.ind() << "</PropertyPartMaterial>" << std::endl;
    
}

void PropertyPartMaterial::Restore(Base::XMLReader &reader)
{
    aboutToSetValue();
    
    reader.readElement("PropertyPartMaterial");
   
    int solidMaterialCount = reader.getAttributeAsInteger("surfaceCount");

    solidMaterials.resize(solidMaterialCount);

    for (int i = 0; i < solidMaterialCount; ++i) {
        std::shared_ptr<MaterialComposition> obj(std::make_shared<MaterialComposition>(this, i));

        obj->Restore(reader);
        solidMaterials.push_back(obj);
    }
    
    int surfaceMaterialCount = reader.getAttributeAsInteger("surfaceCount");

    for (int i = 0; i < surfaceMaterialCount; ++i) {
        reader.readElement("Face");
        
        auto solid = reader.getAttributeAsUnsigned("solid");
        auto face = reader.getAttributeAsUnsigned("face");
        std::shared_ptr<MaterialStack> obj(std::make_shared<MaterialStack>(this));
        
        obj->Restore(reader);

        surfaceMaterials[std::pair<unsigned long, unsigned long>(solid, face)] = obj;

        reader.readEndElement("Face");
    }

    reader.readEndElement("PropertyPartMaterial");

    hasSetValue();
}

MaterialDatabase *PropertyPartMaterial::getDatabase() const
{
    App::DocumentObject * obj = Base::freecad_dynamic_cast<App::DocumentObject>(getContainer());

    if (obj) {
        if (obj->getDocument() == 0)
            throw Base::Exception("The owning object must be attached to a document.");
        else
            return &obj->getDocument()->getMaterialDatabase();
    }
    else
        throw Base::Exception("PropertyPartMaterial must be owned by an App::DocumentObject");
}

namespace Py {

class SolidMaterialArray : public PythonExtension<SolidMaterialArray> {

public:
    SolidMaterialArray(App::PropertyPartMaterial * _owner)
        : owner(_owner)
    {

    }

    static void init_type() {
        behaviors().name("SolidMaterialArray");
        behaviors().doc("SolidMaterialArray class, to support arrays of MaterialComposition objects");
        behaviors().supportSequenceType();
        behaviors().readyType();
    }

    virtual Py::Object sequence_item(Py_ssize_t i) {
        if (!owner->getSolidMaterials(i))
            throw IndexError(boost::str(boost::format("Index %1% out of range") % i));
        return Py::asObject(new MaterialCompositionPy(owner, i));
    }

private:
    App::PropertyPartMaterial * owner;
};

}


Py::PropertyPartMaterial::PropertyPartMaterial(App::PropertyPartMaterial *_owner)
    : PythonExtension<Py::PropertyPartMaterial>()
    , owner(_owner)
{
}

Py::Object Py::PropertyPartMaterial::setSolidMaterial(const Py::Tuple &args)
{
    Py::Int solid(args[0]);
    Py::String materialName(args[1]);

    owner->setSolidMaterial(solid, materialName);

    return Py::None();
}

Py::Object Py::PropertyPartMaterial::getSolidMaterial(const Py::Tuple &args)
{
    Py::Int solid(args[0]);
    Py::Int index(args[1]);

    const App::MaterialComposition * materials = owner->getSolidMaterials(solid);
    return Py::String(materials->get(index));
}

Py::Object Py::PropertyPartMaterial::addSurfaceMaterial(const Py::Tuple &args)
{
    Int solid(args[0]);
    Int face(args[1]);
    String materialName(args[2]);

    owner->addSurfaceMaterial(solid, face, materialName);
    return Py::None();
}

Py::Object Py::PropertyPartMaterial::removeSurfaceMaterial(const Py::Tuple &args)
{
    Int solid(args[0]);
    Int face(args[1]);
    Int index(args[2]);

    owner->removeSurfaceMaterial(solid, face, index);

    return Py::None();
}

Py::Object Py::PropertyPartMaterial::getSurfaceMaterialCount(const Py::Tuple &args)
{
    Int solid(args[0]);
    Int face(args[1]);

    return Py::Int((long)owner->getSurfaceMaterialCount(solid, face));
}

Py::Object Py::PropertyPartMaterial::getSolid(const Py::Tuple &args)
{
    Int solid(args[0]);

    return Py::asObject(new MaterialCompositionPy(owner, solid));
}

Py::Object Py::PropertyPartMaterial::getSurfaceMaterial(const Py::Tuple &args)
{
    Int solid(args[0]);
    Int face(args[1]);
    Int index(args[2]);

    App::MaterialStack * materials = owner->getSurfaceMaterials(solid, face);

    if (materials)
        return Py::asObject(materials->getMaterial(index)->getPyObject());
    else
        throw Py::IndexError("Index out of range");
}

Py::Object Py::PropertyPartMaterial::getattr( const char *name )
{
    if (strcmp(name, "Solid") == 0)
        return Py::asObject(new Py::SolidMaterialArray(owner));
    else {
        return getattr_methods( name );
    }
}

void Py::PropertyPartMaterial::init_type()
{
    SolidMaterialArray::init_type();

    behaviors().name("PropertyPartMaterial");
    behaviors().doc("PropertyPartMaterial class, to support materials for solids");
    behaviors().supportGetattr();

    add_varargs_method("getSolid", &Py::PropertyPartMaterial::getSolid);
    add_varargs_method("setSolidMaterial", &Py::PropertyPartMaterial::setSolidMaterial);
    add_varargs_method("getSolidMaterial", &Py::PropertyPartMaterial::getSolidMaterial);
    add_varargs_method("addSurfaceMaterial", &Py::PropertyPartMaterial::addSurfaceMaterial);
    add_varargs_method("removeSurfaceMaterial", &Py::PropertyPartMaterial::removeSurfaceMaterial);
    add_varargs_method("getSurfaceMaterialCount", &Py::PropertyPartMaterial::getSurfaceMaterialCount);
    add_varargs_method("getSurfaceMaterial", &Py::PropertyPartMaterial::getSurfaceMaterial);

    Py::MaterialDatabasePy    ::init_type();
    Py::MaterialSourcePy      ::init_type();
    Py::MaterialStackPy       ::init_type();
    Py::MaterialCompositionPy ::init_type();
}

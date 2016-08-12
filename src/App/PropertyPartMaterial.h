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

#ifndef PROPERTYSURFACEMATERIAL_H
#define PROPERTYSURFACEMATERIAL_H

#include "Property.h"
#include <memory>
#include <CXX/Extensions.hxx>

namespace App {

class MaterialDatabase;
class MaterialComposition;
class MaterialStack;

class PropertyPartMaterial : public Property
{
    TYPESYSTEM_HEADER();
public:
    PropertyPartMaterial();

    void setValue(const PropertyPartMaterial & value);

    /* Solids */
    void setSolidMaterial(unsigned long solid, const std::string & name);
    void addSolidMaterial(unsigned long solid, const std::string & name, int index = -1);
    void removeSolidMaterial(unsigned long solid, int index);
    size_t getSolidMaterialCount(unsigned long solid) const;
    boost::any getSolidProperty(unsigned long solid, const char * propName) const;
    boost::any getSolidProperty(unsigned long solid, int id) const;
    const MaterialComposition *getSolidMaterials(unsigned long solid) const;

    /* Surfaces */
    void setSurfaceMaterial(unsigned long solid, unsigned long face,  const std::string & name);
    void addSurfaceMaterial(unsigned long solid, unsigned long face, const std::string & name);
    void removeSurfaceMaterial(unsigned long solid, unsigned long face, unsigned long index);
    size_t getSurfaceMaterialCount(unsigned long solid, unsigned long face) const;
    App::MaterialStack *getSurfaceMaterials(unsigned long solid, unsigned long face) const;

    boost::any getSurfaceProperty(unsigned long solid, unsigned long face, unsigned long index, const char * propName) const;
    boost::any getSurfaceProperty(unsigned long solid, unsigned long face, unsigned long index, int id) const;

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);
    
    virtual PyObject *getPyObject(void);
    virtual void setPyObject(PyObject *);

    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);
    
private:
    friend class MaterialStack;
    friend class MaterialComposition;

    App::MaterialDatabase * getDatabase() const;

    Py::Object PythonObject;
    std::vector<std::shared_ptr<MaterialComposition> > solidMaterials;
    std::map<std::pair<unsigned long, unsigned long>, std::shared_ptr<MaterialStack> > surfaceMaterials;
};

}

namespace Py {

class PropertyPartMaterial : public PythonExtension<PropertyPartMaterial> {

public:
    PropertyPartMaterial(App::PropertyPartMaterial * _owner);

    Py::Object setSolidMaterial(const Py::Tuple& args);

    Py::Object getSolidMaterial(const Py::Tuple& args);

    Py::Object addSurfaceMaterial(const Py::Tuple& args);

    Py::Object removeSurfaceMaterial(const Py::Tuple& args);

    Py::Object getSurfaceMaterialCount(const Py::Tuple& args);

    Py::Object getSolid(const Py::Tuple& args);

    Py::Object getSurfaceMaterial(const Py::Tuple& args);

    virtual Py::Object getattr(const char *name);

    static void init_type();

private:
    App::PropertyPartMaterial * owner;
};

}

#endif // PROPERTYSURFACEMATERIAL_H

/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2005     *
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
# include <cstring>
#endif

#include <boost/format.hpp>
#include <assert.h>
#include <Base/Exception.h>
#include "Material.h"
#include "MaterialPy.h"
#include "MaterialSource.h"
#include "MaterialDatabase.h"

using namespace App;

static int ambientColorId = -1;
static int diffuseColorId = -1;
static int specularColorId = -1;
static int emissiveColorId = -1;
static int shininessId = -1;
static int transparencyId = -1;
static int nameId = -1;

//===========================================================================
// Material
//===========================================================================

Material::Material(const MaterialSource * matSource, const std::vector<boost::any> &properties)
 : _matSource(matSource)
 , _matProperties(properties)
{
    assert(matSource != 0);
    setInternalIds();
    PythonObject = new MaterialPy(this);
}

Material::~Material() 
{
}

void Material::setInternalIds()
{
    if (ambientColorId == -1)
        ambientColorId = getPropertyId("AmbientColor");
    if (diffuseColorId == -1)
        diffuseColorId = getPropertyId("DiffuseColor");
    if (specularColorId == -1)
        specularColorId = getPropertyId("SpecularColor");
    if (emissiveColorId == -1)
        emissiveColorId = getPropertyId("EmissiveColor");
    if (shininessId == -1)
        shininessId = getPropertyId("Shininess");
    if (transparencyId == -1)
        transparencyId = getPropertyId("Transparency");
    if (nameId == -1)
        nameId = getPropertyId("Name");
}

const boost::any &Material::getProperty(const char *propName) const
{
    int id = getPropertyId(propName);

    return getProperty(id);
}

void Material::setProperty(const char *propName, const boost::any &value)
{
    setProperty(getPropertyId(propName), value);
}

const char * Material::getPropertyName(int id) const
{
    return _matSource->getPropertyName(id);
}

void Material::setProperty(int id, const boost::any &value)
{
    if (id < 0)
        throw Base::Exception("Cannot set property: Invalid property id");

    if (_matSource->isReadOnly())
        throw Base::Exception(str(boost::format("Unable to set property %1%: material source is read-only.") % getPropertyName(id)));

    if (id >= (int)_matProperties.size())
        _matProperties.resize(id + 1);

    _matProperties[id] = value;
}

void Material::setProperties(const std::vector<boost::any> &properties)
{
    _matProperties = properties;
}

void Material::removeProperty(const char *propName)
{
    removeProperty(getPropertyId(propName));
}

void Material::removeProperty(int id)
{
    if (id < 0)
        throw Base::Exception("Cannot remove property: Invalid property id");
    if ((size_t)id < _matProperties.size())
        _matProperties.resize(id + 1);

    _matProperties[id] = deleted_property_t();
}

int Material::getPropertyId(const char *propName) const
{
    if (_matSource)
        return _matSource->getPropertyId(propName);
    else
        return -1;
}

const boost::any &Material::getProperty(int id) const
{
    if (id < 0) {
        throw Base::Exception("Cannot get property: Invalid property ID");
    }
    else {
        static boost::any empty;
        const boost::any & value = (size_t)id < _matProperties.size() ?_matProperties[id] : empty;

        // If value is empty, ask the Father material
        if (value.empty()) {
            static int fatherId = getPropertyId("Father");
            const boost::any & fatherProp = (size_t)fatherId < _matProperties.size() ?_matProperties[fatherId] : empty;

            if (!fatherProp.empty()) {
                Material * father = getDatabase()->getMaterial(boost::any_cast<std::string>(fatherProp).c_str());

                if (father)
                    return father->getProperty(id);
            }
        }
        else if (value.type() == typeid(deleted_property_t))
            return empty;

        return value;
    }
}

PyObject *Material::getPropertyAsPyObject(const char *propName) const
{
    return _matSource->toPyObject(propName, getProperty(propName));
}

void Material::setPropertyFromPyObject(const char *propName, const PyObject *value)
{
    boost::any anyvalue(_matSource->fromPyObject(propName, value));
    setProperty(propName, anyvalue);
}

bool Material::canUndo() const
{
    return _matSource->canUndo();
}

std::string Material::getName() const
{
    return boost::any_cast<std::string>(_matProperties[nameId]);
}

Color Material::getAmbientColor() const
{
    return boost::any_cast<Color>(getProperty(ambientColorId));
}

void Material::setAmbientColor(const Color &color)
{
    setProperty(ambientColorId, color);
}

Color Material::getDiffuseColor() const
{
    return boost::any_cast<Color>(getProperty(diffuseColorId));
}

void Material::setDiffuseColor(const Color &color)
{
    setProperty(diffuseColorId, color);
}

Color Material::getSpecularColor() const
{
    return boost::any_cast<Color>(getProperty(specularColorId));
}

void Material::setSpecularColor(const Color &color)
{
    setProperty(specularColorId, color);
}

Color Material::getEmissiveColor() const
{
    return boost::any_cast<Color>(getProperty(emissiveColorId));
}

void Material::setEmissiveColor(const Color &color)
{
    setProperty(emissiveColorId, color);
}

float Material::getShininess() const
{
    return boost::any_cast<float>(getProperty(shininessId));
}

void Material::setShininess(float value)
{
    setProperty(shininessId, value);
}

float Material::getTransparency() const
{
    return boost::any_cast<float>(getProperty(transparencyId));
}

void Material::setTransparency(float value)
{
    setProperty(transparencyId, value);
}

PyObject *Material::getPyObject()
{
    return Py::new_reference_to(PythonObject);
}

App::MaterialDatabase *Material::getDatabase() const {
    return _matSource->getDatabase();
}

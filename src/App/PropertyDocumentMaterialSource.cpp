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

#ifndef _PreComp_

#endif

#include "PropertyDocumentMaterialSource.h"
#include "DocumentMaterialSource.h"
#include "Material.h"
#include <Base/Writer.h>
#include <Base/Reader.h>
#include <Base/Tools.h>

using namespace App;

TYPESYSTEM_SOURCE(App::PropertyDocumentMaterialSource, App::Property);

PropertyDocumentMaterialSource::PropertyDocumentMaterialSource()
    : Property()
    , source(std::make_shared<DocumentMaterialSource>())
{
}

void PropertyDocumentMaterialSource::setValue()
{
}

void PropertyDocumentMaterialSource::Save(Base::Writer &writer, const Material * mat) const
{
    const std::vector<boost::any> properties = mat->getProperties();
    size_t n = 0;

    for (auto it = properties.begin(); it != properties.end(); ++it)
        if (!(*it).empty())
            ++n;

    writer.Stream() << writer.ind() << "<Material count=\"" << n << "\">" << std::endl;
    writer.incInd();
    int i = 0;
    for (auto it = properties.begin(); it != properties.end(); ++it, ++i) {
        if (!(*it).empty()) {
            std::string key = source->getPropertyName(i);
            std::string value = source->toString(i, *it);

            if (it->type() == typeid(Material::deleted_property_t))
                writer.Stream() << writer.ind() << "<Property name=\"" << Base::Tools::encodeAttribute(key) << "\" />" << std::endl;
            else
                writer.Stream() << writer.ind() << "<Property name=\"" << Base::Tools::encodeAttribute(key) << "\" value=\"" << Base::Tools::encodeAttribute(value) << "\"/>" << std::endl;
        }
    }
    writer.decInd();
    writer.Stream() << writer.ind() << "</Material>" << std::endl;
}

void PropertyDocumentMaterialSource::Save(Base::Writer &writer) const
{
    writer.Stream() << writer.ind() << "<PropertyDocumentMaterialSource name=\"" << Base::Tools::encodeAttribute(source->getName()) << "\" count=\"" << source->getNumMaterials() << "\">" << std::endl;
    writer.incInd();

    std::vector<Material*> materials;

    source->getMaterials(materials);

    for (auto it = materials.begin(); it != materials.end(); ++it)
        Save(writer, *it);

    writer.decInd();
    writer.Stream() << writer.ind() << "</PropertyDocumentMaterialSource>" << std::endl;
}

void PropertyDocumentMaterialSource::Restore(Base::XMLReader &reader)
{
    reader.readElement("PropertyDocumentMaterialSource");
    // get the value of my Attribute
    int count = reader.getAttributeAsInteger("count");

    for (int i = 0; i < count; i++) {
        reader.readElement("Material");

        int pcount = reader.getAttributeAsInteger("count");
        std::vector<boost::any> properties;

        properties.resize(source->getNumProperties());

        for (int j = 0; j < pcount; ++j) {
            reader.readElement("Property");

            std::string key = reader.getAttribute("key");
            int id = source->getPropertyId(key.c_str());
            boost::any propValue;

            if (reader.hasAttribute("value")) {
                const std::string & value = reader.getAttribute("value");

                if (id >= 0)
                    propValue = source->fromString(key, value);
                else {
                    /* Register unknown property */
                    propValue = source->fromString(key, reader.getAttribute("value"));
                    id = source->registerUnknownProperty(key.c_str());
                    propValue = Material::unknown_property_t(value);
                }
            }
            else
                propValue = Material::deleted_property_t();

            properties[id] = propValue;
        }

        source->createMaterial(properties);

        reader.readEndElement("Material");
    }

    reader.readEndElement("PropertyDocumentMaterialSource");
}

unsigned int PropertyDocumentMaterialSource::getMemSize() const
{
    return 0;
}

Property *PropertyDocumentMaterialSource::Copy() const
{
    PropertyDocumentMaterialSource * obj = new PropertyDocumentMaterialSource();

    obj->source = source->Copy();

    return obj;
}

void PropertyDocumentMaterialSource::Paste(const Property &from)
{
    source->Paste(static_cast<const PropertyDocumentMaterialSource*>(&from)->source);
}

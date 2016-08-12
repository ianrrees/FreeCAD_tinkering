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

#include <Base/Reader.h>
#include <Base/Writer.h>
#include <Base/Tools.h>
#include "Document.h"
#include "DocumentMaterialSource.h"
#include "Material.h"
#include "MaterialDatabase.h"

using namespace App;

DocumentMaterialSource::DocumentMaterialSource()
    : MaterialSource("Document")
{

}

std::shared_ptr<DocumentMaterialSource> DocumentMaterialSource::Copy()
{
    std::shared_ptr<DocumentMaterialSource> obj = std::make_shared<DocumentMaterialSource>();

    obj->name = name;
    obj->database = database;

    // Copy all materials
    for (auto it = material.begin(); it != material.end(); ++it) {
        const std::vector<boost::any> & properties = (*it).second->getProperties();

        obj->createMaterial(properties);
    }

    return obj;
}

void DocumentMaterialSource::Paste(const std::shared_ptr<DocumentMaterialSource> &source)
{
    std::set<std::string> marked;

    name = source->name;
    database = source->database;

    // Mark all materials
    for (auto it = source->material.begin(); it != source->material.end(); ++it)
        marked.insert(it->second->getName());

    for (auto it = source->material.begin(); it != source->material.end(); ++it) {
        const std::string name = it->second->getName();
        auto it2 = material.find(name);

        if (it2 != material.end()) {
            it2->second->setProperties(it->second->getProperties());
            marked.erase(it->second->getName());
        }
        else
            createMaterial(it->second->getProperties());
    }

    // Remove materials that are still marked
    for (auto it = marked.begin(); it != marked.end(); ++it) {
        removeMaterial(material[*it].get());
        ++it;
    }
}

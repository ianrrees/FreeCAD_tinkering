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

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>

#include <Base/Quantity.h>
#include <Base/Tools.h>
#include <Base/Console.h>
#include <Base/Exception.h>
#include "FileMaterialSource.h"
#include "MaterialDatabase.h"

using namespace App;

FileMaterialSource::FileMaterialSource(const char * name, const char * _directory)
    : MaterialSource(name)
    , directory(_directory)
{
    Base::Console().Log("Adding material source %s, from directory %s\n", name, _directory);
}

Material * FileMaterialSource::readMaterial(const std::string & filename) const
{
    std::ifstream f(filename);

    if (f.is_open()) {
        bool matFound(false);
        std::string line;
        std::vector<boost::any> data;
        std::vector<boost::any> properties;

        while (getline(f, line)) {
            if (line.size() > 0 && line[0] == ';')
                data.push_back(line);
            else if (line == "[FCMat]") {
                matFound = true;
                data.push_back(line);
            }
            else if (matFound) {
                auto it = line.find('=');

                if (it != std::string::npos) {
                    std::string key = boost::trim_copy(line.substr(0, it));
                    std::string value = boost::trim_copy(line.substr(it + 1));
                    std::pair<std::string, std::string> pair(key, value);

                    int id = getPropertyId(key.c_str());

                    if (id >= 0) {
                        data.push_back(pair);
                        if (properties.size() <= (size_t)id)
                            properties.resize(id + 1);
                        properties[id] = database->fromString(key, value);
                    }
                    else
                        data.push_back(line);
                }
                else
                    data.push_back(line);
            }
            else
                data.push_back(line);

        }

        int nameId = getPropertyId("Name");

        if (nameId < 0)
            return 0;

        if (properties.size() > 0 && !properties[nameId].empty()) {
            Material * mat = new Material(this, properties);

            return mat;
        }
        else
            return 0;
    }
    else
        return 0;
}

void FileMaterialSource::readDirectory() const
{
    using namespace boost::filesystem;

    path p(directory);
    directory_iterator di(p);
    directory_iterator end;

    while (di != end) {
        directory_entry e(*di);

        if (e.status().type() == regular_file) {
            Material * mat = readMaterial(e.path().generic_string());

            if (mat)
                material[mat->getName()] = std::shared_ptr<Material>(mat);
        }

        ++di;
    }
}

void FileMaterialSource::getMaterials(std::vector<Material *> &materials) const
{
    if (!database)
       throw Base::Exception("Source not added to a database.");

    readDirectory();

    for (auto it = material.begin(); it != material.end(); ++it)
        materials.push_back(it->second.get());
}

void FileMaterialSource::commit()
{
    /* Store materials */
}

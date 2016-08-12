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

#ifndef APP_DOCUMENTMATERIALSOURCE_H
#define APP_DOCUMENTMATERIALSOURCE_H

#include "MaterialSource.h"
#include "Property.h"

namespace Base {
class Reader;
class Writer;
class XMLReader;
}

namespace App {

class Document;

class AppExport DocumentMaterialSource : public MaterialSource
{
//    TYPESYSTEM_HEADER();
public:
    DocumentMaterialSource();

    bool isReadOnly() const { return false; }

    bool canUndo() const { return true; }

    std::shared_ptr<DocumentMaterialSource> Copy();

    void Paste(const std::shared_ptr<DocumentMaterialSource> &source);
};

}

#endif // DOCUMENTMATERIALSOURCE_H

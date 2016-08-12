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

#ifndef APP_PROPERTYDOCUMENTMATERIALSOURCE_H
#define APP_PROPERTYDOCUMENTMATERIALSOURCE_H

#include <memory>

#include "Property.h"

namespace App {

class DocumentMaterialSource;
class Material;

class AppExport PropertyDocumentMaterialSource : public Property
{

    TYPESYSTEM_HEADER();

public:
    PropertyDocumentMaterialSource();

    void setValue();

    std::shared_ptr<DocumentMaterialSource> getValue() const { return source; }

    /// Returns a new copy of the property (mainly for Undo/Redo and transactions)
    virtual Property *Copy(void) const;

    /// Paste the value from the property (mainly for Undo/Redo and transactions)
    virtual void Paste(const Property &from);

    virtual unsigned int getMemSize (void) const;

    virtual void Save (Base::Writer & writer) const;

    virtual void Restore(Base::XMLReader & reader);

protected:
    void Save(Base::Writer &writer, const Material *mat) const;

private:
    std::shared_ptr<DocumentMaterialSource> source;
};

}

#endif // PROPERTYDOCUMENTMATERIALSOURCE_H

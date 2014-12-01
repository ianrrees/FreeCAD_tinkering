/***************************************************************************
 *   Copyright (c) 2014 Luke Parry <l.parry@warwick.ac.uk>                 *
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
# include <sstream>
#endif

#include "Mod/Drawing/App/FeatureTemplate.h"

// inclusion of the generated files (generated out of FeatureTemplateSFPy.xml)
#include "FeatureTemplatePy.h"
#include "FeatureTemplatePy.cpp"

using namespace Drawing;

// returns a string which represents the object e.g. when printed in python
std::string FeatureTemplatePy::representation(void) const
{
    return "<Drawing::FeatureTemplate>";
}

PyObject *FeatureTemplatePy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int FeatureTemplatePy::setCustomAttributes(const char* attr, PyObject* obj)
{
    // search in PropertyList
    App::Property *prop = getFeatureTemplatePtr()->getPropertyByName(attr);
    if (prop) {
        // Read-only attributes must not be set over its Python interface
        short Type =  getFeatureTemplatePtr()->getPropertyType(prop);
        if (Type & App::Prop_ReadOnly) {
            std::stringstream s;
            s << "Object attribute '" << attr << "' is read-only";
            throw Py::AttributeError(s.str());
        }

        prop->setPyObject(obj);
        return 1;
    }

    return 0;
}
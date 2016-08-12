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

#include <memory>
#include <sstream>
#include "Material.h"
#include "MaterialDatabase.h"

#include "DefaultMaterialSource.h"

using namespace App;

DefaultMaterialSource::DefaultMaterialSource()
    : MaterialSource("Legacy visualization materials")
    , isInitializing(false)
{
}

bool DefaultMaterialSource::isReadOnly() const
{
    return !isInitializing;
}

void DefaultMaterialSource::initialize() {
    Material * mat;

    isInitializing = true;

    mat = createMaterial("BRASS");
    mat->setAmbientColor(Color(0.3294f,0.2235f,0.0275f));
    mat->setDiffuseColor(Color(0.7804f,0.5686f,0.1137f));
    mat->setSpecularColor(Color(0.9922f,0.9412f,0.8078f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.2179f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("BRONZE");
    mat->setAmbientColor(Color(0.2125f,0.1275f,0.0540f));
    mat->setDiffuseColor(Color(0.7140f,0.4284f,0.1814f));
    mat->setSpecularColor(Color(0.3935f,0.2719f,0.1667f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.2000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("COPPER");
    mat->setAmbientColor(Color(0.3300f,0.2600f,0.2300f));
    mat->setDiffuseColor(Color(0.5000f,0.1100f,0.0000f));
    mat->setSpecularColor(Color(0.9500f,0.7300f,0.0000f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.9300f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("GOLD");
    mat->setAmbientColor(Color(0.3000f,0.2306f,0.0953f));
    mat->setDiffuseColor(Color(0.4000f,0.2760f,0.0000f));
    mat->setSpecularColor(Color(0.9000f,0.8820f,0.7020f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0625f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("PEWTER");
    mat->setAmbientColor(Color(0.1059f,0.0588f,0.1137f));
    mat->setDiffuseColor(Color(0.4275f,0.4706f,0.5412f));
    mat->setSpecularColor(Color(0.3333f,0.3333f,0.5216f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0769f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("PLASTER");
    mat->setAmbientColor(Color(0.0500f,0.0500f,0.0500f));
    mat->setDiffuseColor(Color(0.1167f,0.1167f,0.1167f));
    mat->setSpecularColor(Color(0.0305f,0.0305f,0.0305f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0078f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("PLASTIC");
    mat->setAmbientColor(Color(0.1000f,0.1000f,0.1000f));
    mat->setDiffuseColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setSpecularColor(Color(0.0600f,0.0600f,0.0600f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0078f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("SILVER");
    mat->setAmbientColor(Color(0.1922f,0.1922f,0.1922f));
    mat->setDiffuseColor(Color(0.5075f,0.5075f,0.5075f));
    mat->setSpecularColor(Color(0.5083f,0.5083f,0.5083f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.2000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("STEEL");
    mat->setAmbientColor(Color(0.0020f,0.0020f,0.0020f));
    mat->setDiffuseColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setSpecularColor(Color(0.9800f,0.9800f,0.9800f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0600f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("STONE");
    mat->setAmbientColor(Color(0.1900f,0.1520f,0.1178f));
    mat->setDiffuseColor(Color(0.7500f,0.6000f,0.4650f));
    mat->setSpecularColor(Color(0.0784f,0.0800f,0.0480f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.1700f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("SHINY_PLASTIC");
    mat->setAmbientColor(Color(0.0880f,0.0880f,0.0880f));
    mat->setDiffuseColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setSpecularColor(Color(1.0000f,1.0000f,1.0000f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 1.0000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("SATIN");
    mat->setAmbientColor(Color(0.0660f,0.0660f,0.0660f));
    mat->setDiffuseColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setSpecularColor(Color(0.4400f,0.4400f,0.4400f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0938f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("METALIZED");
    mat->setAmbientColor(Color(0.1800f,0.1800f,0.1800f));
    mat->setDiffuseColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setSpecularColor(Color(0.4500f,0.4500f,0.4500f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.1300f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("NEON_GNC");
    mat->setAmbientColor(Color(0.2000f,0.2000f,0.2000f));
    mat->setDiffuseColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setSpecularColor(Color(0.6200f,0.6200f,0.6200f));
    mat->setEmissiveColor(Color(1.0000f,1.0000f,0.0000f));
    mat->setShininess( 0.0500f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("CHROME");
    mat->setAmbientColor(Color(0.3500f,0.3500f,0.3500f));
    mat->setDiffuseColor(Color(0.4000f,0.4000f,0.4000f));
    mat->setSpecularColor(Color(0.9746f,0.9746f,0.9746f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.1000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("ALUMINIUM");
    mat->setAmbientColor(Color(0.3000f,0.3000f,0.3000f));
    mat->setDiffuseColor(Color(0.3000f,0.3000f,0.3000f));
    mat->setSpecularColor(Color(0.7000f,0.7000f,0.8000f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0900f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("OBSIDIAN");
    mat->setAmbientColor(Color(0.0538f,0.0500f,0.0662f));
    mat->setDiffuseColor(Color(0.1828f,0.1700f,0.2253f));
    mat->setSpecularColor(Color(0.3327f,0.3286f,0.3464f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.3000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("NEON_PHC");
    mat->setAmbientColor(Color(1.0000f,1.0000f,1.0000f));
    mat->setDiffuseColor(Color(1.0000f,1.0000f,1.0000f));
    mat->setSpecularColor(Color(0.6200f,0.6200f,0.6200f));
    mat->setEmissiveColor(Color(0.0000f,0.9000f,0.4140f));
    mat->setShininess( 0.0500f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("JADE");
    mat->setAmbientColor(Color(0.1350f,0.2225f,0.1575f));
    mat->setDiffuseColor(Color(0.5400f,0.8900f,0.6300f));
    mat->setSpecularColor(Color(0.3162f,0.3162f,0.3162f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.1000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("RUBY");
    mat->setAmbientColor(Color(0.1745f,0.0118f,0.0118f));
    mat->setDiffuseColor(Color(0.6142f,0.0414f,0.0414f));
    mat->setSpecularColor(Color(0.7278f,0.6279f,0.6267f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.6000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("EMERALD");
    mat->setAmbientColor(Color(0.0215f,0.1745f,0.0215f));
    mat->setDiffuseColor(Color(0.0757f,0.6142f,0.0757f));
    mat->setSpecularColor(Color(0.6330f,0.7278f,0.6330f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.6000f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("USER_DEFINED");
    mat->setAmbientColor(Color(0.0020f,0.0020f,0.0020f));
    mat->setDiffuseColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setSpecularColor(Color(0.9800f,0.9800f,0.9800f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess( 0.0600f);
    mat->setTransparency( 0.0000f);

    mat = createMaterial("DEFAULT");
    mat->setAmbientColor(Color(0.2000f,0.2000f,0.2000f));
    mat->setDiffuseColor(Color(0.8000f,0.8000f,0.8000f));
    mat->setSpecularColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setEmissiveColor(Color(0.0000f,0.0000f,0.0000f));
    mat->setShininess(0.2000f);
    mat->setTransparency(0.0000f);

    isInitializing = false;
}

void DefaultMaterialSource::setDatabase(App::MaterialDatabase *database) {
    this->database = database;
    initialize();
}

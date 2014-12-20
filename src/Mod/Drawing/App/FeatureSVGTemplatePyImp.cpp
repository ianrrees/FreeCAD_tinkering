
#include "PreCompiled.h"

#include "Mod/Drawing/App/FeatureSVGTemplate.h"

// inclusion of the generated files (generated out of FeatureSVGTemplatePy.xml)
#include "FeatureSVGTemplatePy.h"
#include "FeatureSVGTemplatePy.cpp"

using namespace Drawing;

// returns a string which represents the object e.g. when printed in python
std::string FeatureSVGTemplatePy::representation(void) const
{
    return std::string("<FeatureSVGTemplate object>");
}

PyObject *FeatureSVGTemplatePy::getCustomAttributes(const char* attr) const
{
    return 0;
}

int FeatureSVGTemplatePy::setCustomAttributes(const char* attr, PyObject* obj)
{
    // search in PropertyList
    App::Property *prop = getFeatureSVGTemplatePtr()->getPropertyByName(attr);
    if (prop) {
        // Read-only attributes must not be set over its Python interface
        short Type =  getFeatureSVGTemplatePtr()->getPropertyType(prop);
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



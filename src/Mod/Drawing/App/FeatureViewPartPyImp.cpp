
#include "PreCompiled.h"

#include "Mod/Drawing/App/FeatureViewPart.h"

// inclusion of the generated files (generated out of FeatureViewPartPy.xml)
#include "FeatureViewPartPy.h"
#include "FeatureViewPartPy.cpp"

using namespace Drawing;

// returns a string which represents the object e.g. when printed in python
std::string FeatureViewPartPy::representation(void) const
{
    return std::string("<FeatureViewPart object>");
}







PyObject *FeatureViewPartPy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int FeatureViewPartPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0; 
}



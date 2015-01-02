
#include "PreCompiled.h"

#include "Mod/Drawing/App/FeatureView.h"

// inclusion of the generated files (generated out of FeatureViewPy.xml)
#include "FeatureViewPy.h"
#include "FeatureViewPy.cpp"

using namespace Drawing;

// returns a string which represents the object e.g. when printed in python
std::string FeatureViewPy::representation(void) const
{
    return std::string("<FeatureView object>");
}







PyObject *FeatureViewPy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int FeatureViewPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0; 
}



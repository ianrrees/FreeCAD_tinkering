
#include "PreCompiled.h"

#include "Mod/Drawing/App/FeatureViewSymbol.h"
#include "Mod/Drawing/App/FeatureView.h"
#include "Mod/Drawing/App/FeatureViewPy.h"

// inclusion of the generated files (generated out of FeatureViewSymbolPy.xml)
#include "FeatureViewSymbolPy.h"
#include "FeatureViewSymbolPy.cpp"

using namespace Drawing;

// returns a string which represents the object e.g. when printed in python
std::string FeatureViewSymbolPy::representation(void) const
{
    return std::string("<FeatureViewSymbol object>");
}







PyObject *FeatureViewSymbolPy::getCustomAttributes(const char* /*attr*/) const
{
    return 0;
}

int FeatureViewSymbolPy::setCustomAttributes(const char* /*attr*/, PyObject* /*obj*/)
{
    return 0; 
}



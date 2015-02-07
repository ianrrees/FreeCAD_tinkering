
#include "PreCompiled.h"
#include <Base/Console.h>

#include "Mod/Drawing/App/FeaturePage.h"
#include "Mod/Drawing/App/FeatureView.h"
#include "FeatureViewPy.h"

// inclusion of the generated files (generated out of FeaturePagePy.xml)
#include "FeaturePagePy.h"
#include "FeaturePagePy.cpp"

using namespace Drawing;

// returns a string which represents the object e.g. when printed in python
std::string FeaturePagePy::representation(void) const
{
    return std::string("<FeaturePage object>");
}

//    int addView(App::DocumentObject *docObj);
PyObject* FeaturePagePy::addView(PyObject* args)
{
    PyObject *pcFeatView;

    if (!PyArg_ParseTuple(args, "O!", &(Drawing::FeatureViewPy::Type), &pcFeatView)) {     // convert args: Python->C
        Base::Console().Error("Error: FeaturePagePy::addView - Bad Args\n");
        return NULL;                             // NULL triggers exception
    }

    PyErr_SetString(PyExc_NotImplementedError, "Not yet implemented");
    return 0;
}

//    double getPageWidth() const;
PyObject* FeaturePagePy::getPageWidth(PyObject *args)
{
    PyErr_SetString(PyExc_NotImplementedError, "Not yet implemented");
    return 0;
}

//    double getPageHeight() const;
PyObject* FeaturePagePy::getPageHeight(PyObject *args)
{
    PyErr_SetString(PyExc_NotImplementedError, "Not yet implemented");
    return 0;
}

//    const char* getPageOrientation() const;
PyObject* FeaturePagePy::getPageOrientation(PyObject *args)
{
    PyErr_SetString(PyExc_NotImplementedError, "Not yet implemented");
    return 0;
}

PyObject *FeaturePagePy::getCustomAttributes(const char* attr) const
{
    return 0;
}

int FeaturePagePy::setCustomAttributes(const char* attr, PyObject *obj)
{
    return 0; 
}



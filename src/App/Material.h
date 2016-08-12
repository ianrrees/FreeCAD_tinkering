/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2005     *
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


#ifndef APP_MATERIAL_H
#define APP_MATERIAL_H

#ifdef __GNUC__
# include <stdint.h>
#endif

#include <sstream>
#include <iomanip>
#include <memory>
#include <boost/any.hpp>
#include <boost/noncopyable.hpp>

#include <Base/BaseClass.h>
#include <CXX/Objects.hxx>

namespace App
{

class MaterialSource;
class MaterialDatabase;

/** Color class
 */
class AppExport Color
{
public:
    /**
     * Defines the color as (R,G,B,A) whereas all values are in the range [0,1].
     * \a A defines the transparency.
     */
    explicit Color(float R=0.0,float G=0.0, float B=0.0, float A=0.0)
      :r(R),g(G),b(B),a(A){}
    /**
     * Does basically the same as the constructor above unless that (R,G,B,A) is
     * encoded as an unsigned int.
     */
    explicit Color(uint32_t rgba)
    { setPackedValue( rgba ); }

    /** Copy constructor. */
    Color(const Color& c)
      :r(c.r),g(c.g),b(c.b),a(c.a){}

    /** Returns true if both colors are equal. Therefore all components must be equal. */
    bool operator==(const Color& c) const
    {
        return getPackedValue() == c.getPackedValue();
        //return (c.r==r && c.g==g && c.b==b && c.a==a);
    }
    bool operator!=(const Color& c) const
    {
        return !operator==(c);
    }
    /**
     * Defines the color as (R,G,B,A) whereas all values are in the range [0,1].
     * \a A defines the transparency, 0 means complete opaque and 1 invisible.
     */
    void set(float R,float G, float B, float A=0.0)
    {
        r=R;g=G;b=B;a=A;
    }
    Color& operator=(const Color& c)
    {
        r=c.r;g=c.g;b=c.b;a=c.a;
        return *this;
    }
    /**
     * Sets the color value as a 32 bit combined red/green/blue/alpha value.
     * Each component is 8 bit wide (i.e. from 0x00 to 0xff), and the red
     * value should be stored leftmost, like this: 0xRRGGBBAA.
     *
     * \sa getPackedValue().
     */
    Color& setPackedValue(uint32_t rgba)
    {
        this->set((rgba >> 24)/255.0f,
                 ((rgba >> 16)&0xff)/255.0f,
                 ((rgba >> 8)&0xff)/255.0f,
                 (rgba&0xff)/255.0f);
        return *this;
    }
    /**
     * Returns color as a 32 bit packed unsigned int in the form 0xRRGGBBAA.
     *
     *  \sa setPackedValue().
     */
    uint32_t getPackedValue() const
    {
        return ((uint32_t)(r*255.0f + 0.5f) << 24 |
                (uint32_t)(g*255.0f + 0.5f) << 16 |
                (uint32_t)(b*255.0f + 0.5f) << 8  |
                (uint32_t)(a*255.0f + 0.5f));
    }
    /**
     * creates FC Color from template type, e.g. Qt QColor
     */
    template <typename T>
    void setValue(const T& q)
    { set(q.redF(),q.greenF(),q.blueF()); }
    /**
     * returns a template type e.g. Qt color equivalent to FC color
     *
     */
    template <typename T>
    inline T asValue(void) const {
        return(T(int(r*255.0),int(g*255.0),int(b*255.0)));
    }
    /**
     * returns color as CSS color "#RRGGBB"
     *
     */
    std::string asCSSString() {
        std::stringstream ss;
        ss << "#" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << int(r*255.0)
                                                                     << std::setw(2) << int(g*255.0)
                                                                     << std::setw(2) << int(b*255.0);
        return ss.str();
}

    /// color values, public accessible
    float r,g,b,a;
};

/** Material class
 */
class AppExport Material : private boost::noncopyable
{
public:
    /** @name Constructors
     */
    //@{
    /** Create a material from a set of properties */

    Material(const App::MaterialSource * _matSource, const std::vector<boost::any> & properties);

    //@}
    ~Material();

    // Special struct to signify a removed property
    typedef struct deleted_property_s {
        deleted_property_s() { }
    } deleted_property_t;


    // Special struct to signify a removed property
    typedef struct unknown_property_s {
        unknown_property_s(const std::string & s) : value(s) { }
    private:
        std::string value;
    } unknown_property_t;

    // Getters

    int getPropertyId(const char * propName) const;

    const boost::any & getProperty(const char * propName) const;

    const boost::any & getProperty(int id) const;

    const std::vector<boost::any> & getProperties() const { return _matProperties; }

    // Setters

    void setProperty(const char * propName, const boost::any & value);

    void setProperty(int id, const boost::any & value);

    void setProperties(const std::vector<boost::any> & properties);

    // Remove property

    void removeProperty(const char * propName);

    void removeProperty(int id);

    // Whether changes to this material can be undone or not

    bool canUndo() const;

    // Convenience function to access various proerties

    std::string getName() const;

    Color getAmbientColor() const;
    void setAmbientColor(const Color & color);

    Color getDiffuseColor() const;
    void setDiffuseColor(const Color & color);

    Color getSpecularColor() const;
    void setSpecularColor(const Color & color);

    Color getEmissiveColor() const;
    void setEmissiveColor(const Color & color);

    float getShininess() const;
    void setShininess(float value);

    float getTransparency() const;
    void setTransparency(float value);

    virtual PyObject *getPyObject(void);

private:

    friend class MaterialPy;

    PyObject * getPropertyAsPyObject(const char * propName) const;

    void setPropertyFromPyObject(const char * propName, const PyObject * value);

    const char *getPropertyName(int id) const;

    App::MaterialDatabase * getDatabase() const;

    void setInternalIds();

    Py::Object PythonObject;
    const App::MaterialSource * _matSource;
    std::vector<boost::any> _matProperties;
};

} //namespace App

#endif // APP_MATERIAL_H

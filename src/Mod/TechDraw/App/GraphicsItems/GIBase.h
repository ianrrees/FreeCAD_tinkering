/***************************************************************************
 *   Copyright (c) 2016                    Ian Rees <ian.rees@gmail.com>   *
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

#ifndef GIBASE_HEADER
#define GIBASE_HEADER

#include <QFont>
#include <QGraphicsItemGroup>
#include <QObject>
#include <QPen>

#include "../DrawView.h"

// Allows declaring QGIView as a friend of GIBase, even if it's not present.
namespace TechDrawGui {
    class QGIView;
}

namespace TechDraw {

/// Base class for TechDraw classes that derive from QGraphicsItemGroup
/*!
 * In order to accommodate a split between the GUI and non-GUI code, we
 * implement all the non-interactive QGraphics in this directory and the GUI
 * specific code in the ../../Gui directory.  Classes in the GUI side derive
 * from classes here, which creates a ladder-like pattern.  As an example:
 *
 * Non-GUI:             GUI:
 * GIBase        <-   QGIView
 *   ^                   ^
 * GICollection  <-  QGIViewCollection
 *   ^                   ^
 * GIProjGroup   <-  QGIProjGroup
 *
 * This creates a few issues to be mindful of.  For example, the itemChange()
 * method of QGraphicsItem is overridden in GIBase and QGIView, and should not
 * be used directly.  Instead, the Non-GUI side implements
 * graphicsItemChange(), and the GUI side implements guiGraphicsItemChange().
 * This way, events can be passed through each class exactly once.
 */
class TechDrawExport GIBase : public QGraphicsItemGroup
{
public:
    GIBase();
    virtual ~GIBase() = default;

    enum {Type = QGraphicsItem::UserType + 101};
    virtual int type() const { return Type;}

    const char * getViewName() const;

    void setViewFeature(TechDraw::DrawView *obj);
    TechDraw::DrawView * getViewObject() const;    

    /// Methods to ensure that Y-Coordinates are orientated correctly.
    /// @{
    void setPosition(qreal x, qreal y);
    inline qreal getY() { return y() * -1; }
    bool isInnerView() { return m_innerView; }
    void isInnerView(bool state) { m_innerView = state; }
    /// @}

    /// Used for constraining views to line up eg in a Projection Group
    void alignTo(QGraphicsItem*, const QString &alignment);

    virtual void updateView(bool update = false);
    virtual void paint( QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget = nullptr );

protected:
    double getYInClip(double y);

    DrawView *viewObj;

    bool locked;
    bool m_innerView;  //View is inside another View

    std::string viewName;

    QColor m_colCurrent;
    QColor m_colNormal;
    QColor m_colPre;
    QColor m_colSel;
    QFont m_font;
    QGraphicsRectItem* m_border;
    QGraphicsTextItem* m_label;
    QHash<QString, QGraphicsItem*> alignHash;
    QPen m_pen;

    /// Used for our non-GUI children to replicate QGraphicsItem::itemChange()
    /*!
     * If derived classes need to override this, they should finish by calling
     * the corresponding method of their non-GUI GIBase-derived parent only.
     */
    virtual QVariant graphicsItemChange(GraphicsItemChange change, const QVariant &value);

private:
    // This pattern is meant to let TechDrawGui::QGIView (if it is compied in)
    // declare itemChange() final, so that we can resolve confusing multiple-
    // inheritace situations.
    friend class TechDrawGui::QGIView;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

} // end namespace TechDraw

#endif // #ifndef GIBASE_HEADER


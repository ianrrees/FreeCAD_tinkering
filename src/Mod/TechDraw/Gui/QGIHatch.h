/***************************************************************************
 *   Copyright (c) 2015 WandererFan <wandererfan@gmail.com>                *
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

#ifndef DRAWINGGUI_QGRAPHICSITEMHATCH_H
#define DRAWINGGUI_QGRAPHICSITEMHATCH_H

#include <QBitmap>

#include "../App/GraphicsItems/GIHatch.h"

namespace App {
    class Color;
}

namespace TechDrawGui
{

class TechDrawGuiExport QGIHatch : public TechDraw::GIHatch
{
public:
    QGIHatch(std::string parentHatch);
    ~QGIHatch() = default;

    std::string getHatchName() const { return m_hatch; }
    void setPrettyNormal();
    void setPrettyPre();
    void setPrettySel();
    void setFill(std::string fillSpec);
    void setColor(App::Color c);

protected:
    // Preselection events:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    // Selection detection
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    QColor m_colPre;
    QColor m_colSel;
    QBitmap m_texture;
    std::string m_lastFill;
};

} // namespace TechDrawGui

#endif // DRAWINGGUI_QGRAPHICSITEMHATCH_H

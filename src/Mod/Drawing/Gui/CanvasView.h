/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
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

#ifndef DRAWINGGUI_CANVASVIEW_H
#define DRAWINGGUI_CANVASVIEW_H

#include <QGraphicsView>

namespace Drawing {
class FeatureViewPart;
class FeatureViewOrthographic;
class FeatureViewDimension;
class FeaturePage;
class FeatureTemplate;
class FeatureViewAnnotation;
class FeatureViewSymbol;
}

namespace DrawingGui
{
class QGraphicsItemView;
class QGraphicsItemViewDimension;
class QGraphicsItemTemplate;
class ViewProviderDrawingPage;

class DrawingGuiExport CanvasView : public QGraphicsView
{
    Q_OBJECT

public:
    enum RendererType { Native, OpenGL, Image };

    CanvasView(ViewProviderDrawingPage *vp, QWidget *parent = 0);
    ~CanvasView();

    void setRenderer(RendererType type = Native);
    void drawBackground(QPainter *p, const QRectF &rect);

    QGraphicsItemView * addViewDimension(Drawing::FeatureViewDimension *dim);
    QGraphicsItemView * addViewOrthographic(Drawing::FeatureViewOrthographic *view);
    QGraphicsItemView * addViewPart(Drawing::FeatureViewPart *part);
    QGraphicsItemView * addViewSection(Drawing::FeatureViewPart *part);
    QGraphicsItemView * addFeatureView(Drawing::FeatureView *view);
    QGraphicsItemView * addFeatureViewCollection(Drawing::FeatureViewCollection *view);
    QGraphicsItemView * addFeatureViewAnnotation(Drawing::FeatureViewAnnotation *view);
    QGraphicsItemView * addFeatureViewSymbol(Drawing::FeatureViewSymbol *view);

    QGraphicsItemView * findView(App::DocumentObject *obj) const;
    QGraphicsItemView * findParent(QGraphicsItemView *) const;

    void addDimToParent(QGraphicsItemViewDimension* dim, QGraphicsItemView* parent);
    
    const std::vector<QGraphicsItemView *> & getViews() const { return views; }
    int addView(QGraphicsItemView * view);
    void setViews(const std::vector<QGraphicsItemView *> &view) {views = view; }
    void setPageFeature(Drawing::FeaturePage *page);
    void setPageTemplate(Drawing::FeatureTemplate *pageTemplate);

    QGraphicsItemTemplate * getTemplate() const;
    void removeTemplate();

    void toggleEdit(bool enable);


public Q_SLOTS:
    void setHighQualityAntialiasing(bool highQualityAntialiasing);
    void setViewBackground(bool enable);
    void setViewOutline(bool enable);

protected:
    void wheelEvent(QWheelEvent *event);
    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    static QColor SelectColor;
    static QColor PreselectColor;

    QGraphicsItemTemplate *pageTemplate;
    std::vector<QGraphicsItemView *> views;

private:
    RendererType m_renderer;

    bool drawBkg;
    Drawing::FeaturePage *pageFeat;
    QGraphicsRectItem *m_backgroundItem;
    QGraphicsRectItem *m_outlineItem;
    QBrush *bkgBrush;
    QImage m_image;
    ViewProviderDrawingPage *pageGui;
};

} // namespace DrawingViewGui

#endif // DRAWINGGUI_CANVASVIEW_H

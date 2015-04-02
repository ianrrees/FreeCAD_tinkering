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

#include "PreCompiled.h"
#ifndef _PreComp_
# include <QAction>
# include <QApplication>
# include <QContextMenuEvent>
# include <QFileInfo>
# include <QFileDialog>
# include <QGLWidget>
# include <QGraphicsScene>
# include <QGraphicsSvgItem>
# include <QGraphicsEffect>
# include <QMouseEvent>
# include <QPainter>
# include <QPaintEvent>
# include <QSvgRenderer>
# include <QSvgWidget>
# include <QWheelEvent>
# include <strstream>
# include <cmath>
#endif

#include <Base/Console.h>
#include <Base/Stream.h>
#include <Gui/FileDialog.h>
#include <Gui/WaitCursor.h>

#include <Mod/Drawing/App/Geometry.h>
#include <Mod/Drawing/App/FeaturePage.h>
#include <Mod/Drawing/App/FeatureTemplate.h>
#include <Mod/Drawing/App/FeatureSVGTemplate.h>
#include <Mod/Drawing/App/FeatureParametricTemplate.h>
#include <Mod/Drawing/App/FeatureViewCollection.h>
#include <Mod/Drawing/App/FeatureViewDimension.h>
#include <Mod/Drawing/App/FeatureViewOrthographic.h>
#include <Mod/Drawing/App/FeatureViewPart.h>
#include <Mod/Drawing/App/FeatureViewAnnotation.h>
#include <Mod/Drawing/App/FeatureViewSymbol.h>

#include "ViewProviderPage.h"

#include "QGraphicsItemDrawingTemplate.h"
#include "QGraphicsItemSVGTemplate.h"
#include "QGraphicsItemViewCollection.h"
#include "QGraphicsItemViewDimension.h"
#include "QGraphicsItemViewOrthographic.h"
#include "QGraphicsItemViewPart.h"
#include "QGraphicsItemViewSection.h"
#include "QGraphicsItemViewAnnotation.h"
#include "QGraphicsItemViewSymbol.h"


#include "CanvasView.h"

using namespace DrawingGui;
QColor CanvasView::PreselectColor   =    QColor(0.88f,0.88f,0.0f);   // #E1E100 -> (225,225,  0)
QColor CanvasView::SelectColor      =    QColor(0.11f,0.68f,0.11f);  // #1CAD1C -> ( 28,173, 28)

CanvasView::CanvasView(ViewProviderDrawingPage *vp, QWidget *parent)
    : pageGui(0)
    , QGraphicsView(parent)
    , m_renderer(Native)
    , m_backgroundItem(0)
    , m_outlineItem(0)
    , drawBkg(true)
    , pageTemplate(0)
{
    assert(vp);
    pageGui = vp;
    const char* name = vp->getPageObject()->getNameInDocument();
    this->setObjectName(QString::fromLocal8Bit(name));

    setScene(new QGraphicsScene(this));
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    setTransformationAnchor(AnchorUnderMouse);

    setDragMode(ScrollHandDrag);
    setCursor(QCursor(Qt::ArrowCursor));
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    m_backgroundItem = new QGraphicsRectItem();
    m_backgroundItem->setCacheMode(QGraphicsItem::NoCache);
    m_backgroundItem->setZValue(-999999);
//     this->scene()->addItem(m_backgroundItem); // TODO IF SEGFAULTS WITH DRAW ENABLE THIS (REDRAWS ARE SLOWER :s)

    // Prepare background check-board pattern
    QLinearGradient gradient;
    gradient.setStart(0, 0);
    gradient.setFinalStop(0, this->height());
    gradient.setColorAt(0., QColor(72, 72, 72));
    gradient.setColorAt(1., QColor(150, 150, 150));
    bkgBrush = new QBrush(QColor::fromRgb(70,70,70));

    resetCachedContent();
}
CanvasView::~CanvasView()
{
    delete bkgBrush;
    delete m_backgroundItem;
}

void CanvasView::drawBackground(QPainter *p, const QRectF &)
{
    if(!this->drawBkg)
        return;

    p->save();
    p->resetTransform();


    p->setBrush(*bkgBrush);
    p->drawRect(viewport()->rect());

    if(!pageGui) {
        return;
    }

    float pageWidth  = pageGui->getPageObject()->getPageWidth();
    float pageHeight = pageGui->getPageObject()->getPageHeight();

    // Draw the white page
    QRectF paperRect(0, -pageHeight, pageWidth, pageHeight);
    QPolygon poly = this->mapFromScene(paperRect);

    QBrush pageBrush(Qt::white);
    p->setBrush(pageBrush);

    p->drawRect(poly.boundingRect());

    p->restore();

}

int CanvasView::addView(QGraphicsItemView * view) {
    views.push_back(view);

    // Find if it belongs to a parent
    QGraphicsItemView *parent = 0;
    parent = this->findParent(view);

    QPointF viewPos(view->getViewObject()->X.getValue(),
                    view->getViewObject()->Y.getValue() * -1);

    if(parent) {
        // Transfer the child vierw to the parent
        QPointF posRef(0.,0.);

        QPointF mapPos = view->mapToItem(parent, posRef);
        view->moveBy(-mapPos.x(), -mapPos.y());

        parent->addToGroup(view);
    }

    view->setPos(viewPos);

    return views.size();
}

QGraphicsItemView * CanvasView::addViewPart(Drawing::FeatureViewPart *part)
{
    QGraphicsItemViewPart *viewPart = new QGraphicsItemViewPart(QPoint(0,0), this->scene());
    viewPart->setViewPartFeature(part);

    this->addView(viewPart);
    return viewPart;
}

QGraphicsItemView * CanvasView::addViewSection(Drawing::FeatureViewPart *part)
{
    QGraphicsItemViewSection *viewSection = new QGraphicsItemViewSection(QPoint(0,0), this->scene());
    viewSection->setViewPartFeature(part);

    this->addView(viewSection);
    return viewSection;
}

QGraphicsItemView * CanvasView::addViewOrthographic(Drawing::FeatureViewOrthographic *view) {
    QGraphicsItemViewCollection *qview = new  QGraphicsItemViewOrthographic(QPoint(0,0), this->scene());
    qview->setViewFeature(view);
    this->addView(qview);
    return qview;
}

QGraphicsItemView * CanvasView::addFeatureView(Drawing::FeatureView *view)
{
    QGraphicsItemView *qview = new  QGraphicsItemView(QPoint(0,0), this->scene());
    qview->setViewFeature(view);
    this->addView(qview);
    return qview;
}

QGraphicsItemView * CanvasView::addFeatureViewCollection(Drawing::FeatureViewCollection *view)
{
    QGraphicsItemViewCollection *qview = new  QGraphicsItemViewCollection(QPoint(0,0), this->scene());
    qview->setViewFeature(view);
    this->addView(qview);
    return qview;
}

// TODO change to (App?) annotation object  ??
QGraphicsItemView * CanvasView::addFeatureViewAnnotation(Drawing::FeatureViewAnnotation *view)
{
    //QPoint qp(view->X.getValue(),view->Y.getValue());
    // This essentially adds a null view feature to ensure view size is consistent
    //QGraphicsItemViewAnnotation *qview = new  QGraphicsItemViewAnnotation(qp, this->scene());
    QGraphicsItemViewAnnotation *qview = new  QGraphicsItemViewAnnotation(QPoint(0,0), this->scene());
    qview->setViewAnnoFeature(view);

    this->addView(qview);
    return qview;
}

QGraphicsItemView * CanvasView::addFeatureViewSymbol(Drawing::FeatureViewSymbol *view)
{
    QPoint qp(view->X.getValue(),view->Y.getValue());
    // This essentially adds a null view feature to ensure view size is consistent
    QGraphicsItemViewSymbol *qview = new  QGraphicsItemViewSymbol(qp, this->scene());
    qview->setViewFeature(view);

    this->addView(qview);
    return qview;
}

QGraphicsItemView * CanvasView::addViewDimension(Drawing::FeatureViewDimension *dim)
{
    QGraphicsItemViewDimension *dimGroup = new QGraphicsItemViewDimension(QPoint(0,0), this->scene());
    dimGroup->setViewPartFeature(dim);

    // TODO consider changing dimension feature to use another property for label position
    // Instead of calling addView - the view must for now be added manually

    views.push_back(dimGroup);

    // Find if it belongs to a parent
    QGraphicsItemView *parent = 0;
    parent = this->findParent(dimGroup);

    if(parent) {
        // Transfer the child vierw to the parent
        QPointF posRef(0.,0.);
        QPointF mapPos = dimGroup->mapToItem(parent, posRef);
        dimGroup->moveBy(-mapPos.x(), -mapPos.y());

        parent->addToGroup(dimGroup);
    }

    return dimGroup;
}

QGraphicsItemView * CanvasView::findView(App::DocumentObject *obj) const
{
  if(scene()) {
    const std::vector<QGraphicsItemView *> qviews = this->views;
    for(std::vector<QGraphicsItemView *>::const_iterator it = qviews.begin(); it != qviews.end(); ++it) {
          Drawing::FeatureView *fview = (*it)->getViewObject();
          if(fview && strcmp(obj->getNameInDocument(), fview->getNameInDocument()) == 0)
              return *it;
      }
  }
    return 0;
}

QGraphicsItemView * CanvasView::findParent(QGraphicsItemView *view) const
{
    const std::vector<QGraphicsItemView *> qviews = this->views;
    Drawing::FeatureView *myView = view->getViewObject();

    //If type is dimension we check references first
    // Todo change this to an annotation object later
    Drawing::FeatureViewDimension *dim = 0;
    dim = dynamic_cast<Drawing::FeatureViewDimension *>(myView);

    if(dim) {
        std::vector<App::DocumentObject *> objs = dim->References.getValues();

        if(objs.size() > 0) {
            std::vector<App::DocumentObject *> objs = dim->References.getValues();
            // Attach the dimension to the first object's group
            for(std::vector<QGraphicsItemView *>::const_iterator it = qviews.begin(); it != qviews.end(); ++it) {
                Drawing::FeatureView *viewObj = (*it)->getViewObject();
                if(strcmp(viewObj->getNameInDocument(), objs.at(0)->getNameInDocument()) == 0) {
                    return *it;
                }
            }
        }
    }

    // Check if part of view collection
    for(std::vector<QGraphicsItemView *>::const_iterator it = qviews.begin(); it != qviews.end(); ++it) {
        QGraphicsItemViewCollection *grp = 0;
        grp = dynamic_cast<QGraphicsItemViewCollection *>(*it);
        if(grp) {
            Drawing::FeatureViewCollection *collection = 0;
            collection = dynamic_cast<Drawing::FeatureViewCollection *>(grp->getViewObject());
            if(collection) {
                std::vector<App::DocumentObject *> objs = collection->Views.getValues();
                for( std::vector<App::DocumentObject *>::iterator it = objs.begin(); it != objs.end(); ++it) {
                    if(strcmp(myView->getNameInDocument(), (*it)->getNameInDocument()) == 0)

                        return grp;
                }
            }
        }
     }

    // Not found a parent
    return 0;
}

void CanvasView::setPageFeature(Drawing::FeaturePage *page)
{
    //redundant
#if 0
    // TODO verify if the pointer should even be used. Not really safe
    this->pageFeat = page;

    float pageWidth  = pageGui->getPageObject()->getPageWidth();
    float pageHeight = pageGui->getPageObject()->getPageHeight();

    QRectF paperRect(0, -pageHeight, pageWidth, pageHeight);

    QBrush brush(Qt::white);

    m_backgroundItem->setBrush(brush);
    m_backgroundItem->setRect(paperRect);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10.0);
    shadow->setColor(Qt::white);
    shadow->setOffset(0,0);
    m_backgroundItem->setGraphicsEffect(shadow);

    QRectF myRect = paperRect;
    myRect.adjust(-20,-20,20,20);
    this->setSceneRect(myRect);
#endif
}

void CanvasView::setPageTemplate(Drawing::FeatureTemplate *obj)
{
    // Remove currently set background template
    // Assign a base template class and create object dependent on
    this->removeTemplate();

    if(obj->isDerivedFrom(Drawing::FeatureParametricTemplate::getClassTypeId())) {
        Drawing::FeatureParametricTemplate *dwgTemplate = static_cast<Drawing::FeatureParametricTemplate *>(obj);
        QGraphicsItemDrawingTemplate *qTempItem = new QGraphicsItemDrawingTemplate(this->scene());
        qTempItem->setTemplate(dwgTemplate);
        qTempItem->updateView();
        this->pageTemplate = qTempItem;
    } else if(obj->isDerivedFrom(Drawing::FeatureSVGTemplate::getClassTypeId())) {
        Drawing::FeatureSVGTemplate *dwgTemplate = static_cast<Drawing::FeatureSVGTemplate *>(obj);
        QGraphicsItemSVGTemplate *qTempItem = new QGraphicsItemSVGTemplate(this->scene());
        qTempItem->setTemplate(dwgTemplate);
        qTempItem->updateView();
        this->pageTemplate = qTempItem;
    }
}

QGraphicsItemTemplate* CanvasView::getTemplate() const
{
    return pageTemplate;
}

void CanvasView::removeTemplate()
{
    if(this->pageTemplate) {
        this->scene()->removeItem(this->pageTemplate);
        this->pageTemplate->deleteLater();
        this->pageTemplate = 0;
    }
}
void CanvasView::setRenderer(RendererType type)
{
    m_renderer = type;

    if (m_renderer == OpenGL) {
#ifndef QT_NO_OPENGL
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif
    } else {
        setViewport(new QWidget);
    }
}

void CanvasView::setHighQualityAntialiasing(bool highQualityAntialiasing)
{
#ifndef QT_NO_OPENGL
    setRenderHint(QPainter::HighQualityAntialiasing, highQualityAntialiasing);
#else
    Q_UNUSED(highQualityAntialiasing);
#endif
}

void CanvasView::setViewBackground(bool enable)
{
    if (!m_backgroundItem)
        return;

    m_backgroundItem->setVisible(enable);
}

void CanvasView::setViewOutline(bool enable)
{
    if (!m_outlineItem)
        return;

    m_outlineItem->setVisible(enable);
}

void CanvasView::toggleEdit(bool enable)
{
// TODO: needs fiddling to handle items that don't inherit QGraphicsItemViewPart: Annotation, Symbol, Templates, Edges, Faces, Vertices,...
    QList<QGraphicsItem *> list = this->scene()->items();

    for (QList<QGraphicsItem *>::iterator it = list.begin(); it != list.end(); ++it) {
        QGraphicsItemView *itemView = dynamic_cast<QGraphicsItemView *>(*it);
        if(itemView) {
            QGraphicsItemViewPart *viewPart = dynamic_cast<QGraphicsItemViewPart *>(*it);
            itemView->setSelected(false);
            if(viewPart) {
                viewPart->toggleCache(enable);
                viewPart->toggleCosmeticLines(enable);
                viewPart->toggleVertices(enable);
                viewPart->toggleBorder(enable);
                setViewBackground(enable);
            } else {
                itemView->toggleBorder(enable); 
            }
            //itemView->updateView(true);
        }

        QGraphicsItem *item = dynamic_cast<QGraphicsItem *>(*it);
        if(item) {
            //item->setCacheMode((enable) ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);
            item->setCacheMode((enable) ? QGraphicsItem::NoCache : QGraphicsItem::NoCache);
            item->update();
        }
    }
    this->scene()->update();
    this->update();
    this->viewport()->repaint();
}


void CanvasView::paintEvent(QPaintEvent *event)
{
    if (m_renderer == Image) {
        if (m_image.size() != viewport()->size()) {
            m_image = QImage(viewport()->size(), QImage::Format_ARGB32_Premultiplied);
        }

        QPainter imagePainter(&m_image);
        QGraphicsView::render(&imagePainter);
        imagePainter.end();

        QPainter p(viewport());
        p.drawImage(0, 0, m_image);

    } else {
        QGraphicsView::paintEvent(event);
    }
}

void CanvasView::wheelEvent(QWheelEvent *event)
{
    qreal factor = std::pow(1.2, -event->delta() / 240.0);
    scale(factor, factor);
    event->accept();
}
void CanvasView::enterEvent(QEvent *event)
{
    QGraphicsView::enterEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    viewport()->setCursor(Qt::ClosedHandCursor);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}

#include "moc_CanvasView.cpp"

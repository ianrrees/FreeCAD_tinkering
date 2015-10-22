/***************************************************************************
 *   Copyright (c) 2007 Jürgen Riegel <juergen.riegel@web.de>              *
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
# include <QBuffer>
# include <QContextMenuEvent>
# include <QFileInfo>
# include <QFileDialog>
# include <QGLWidget>
# include <QGraphicsRectItem>
# include <QGraphicsSvgItem>
# include <QGridLayout>
# include <QGroupBox>
# include <QListWidget>
# include <QMenu>
# include <QMessageBox>
# include <QMouseEvent>
# include <QPainter>
# include <QPaintEvent>
# include <QPrinter>
# include <QPrintDialog>
# include <QPrintPreviewDialog>
# include <QPrintPreviewWidget>
# include <QScrollArea>
# include <QSlider>
# include <QStatusBar>
# include <QSvgRenderer>
# include <QSvgGenerator>
# include <QSvgWidget>
# include <QWheelEvent>
# include <strstream>
# include <cmath>
#endif  // #ifndef _PreComp_

#include "DrawingView.h"

#include <Base/Stream.h>
#include <Base/gzstream.h>
#include <Base/PyObjectBase.h>
#include <Base/Console.h>

#include <App/Document.h>
#include <App/DocumentObject.h>

#include <Gui/Document.h>
#include <Gui/ViewProvider.h>
#include <Gui/FileDialog.h>
#include <Gui/WaitCursor.h>
#include <Gui/Application.h>
#include <Gui/Command.h>
#include <Gui/Document.h>
#include <Gui/Window.h>
#include <Gui/MainWindow.h>
#include <Gui/Selection.h>

#include <Mod/Drawing/App/FeaturePage.h>
#include <Mod/Drawing/App/FeatureView.h>
#include <Mod/Drawing/App/FeatureViewCollection.h>
#include <Mod/Drawing/App/FeatureProjGroup.h>
#include <Mod/Drawing/App/FeatureViewPart.h>
#include <Mod/Drawing/App/FeatureViewSection.h>
#include <Mod/Drawing/App/FeatureViewDimension.h>
#include <Mod/Drawing/App/FeatureTemplate.h>
#include <Mod/Drawing/App/FeatureViewAnnotation.h>
#include <Mod/Drawing/App/FeatureViewSymbol.h>
#include <Mod/Drawing/App/FeatureViewClip.h>

#include "QGraphicsItemDrawingTemplate.h"
#include "QGraphicsItemView.h"
#include "QGraphicsItemViewPart.h"
#include "QGraphicsItemViewDimension.h"
#include "QGraphicsItemViewClip.h"
#include "ViewProviderPage.h"
#include "CanvasView.h"


using namespace DrawingGui;

/* TRANSLATOR DrawingGui::DrawingView */

DrawingView::DrawingView(ViewProviderDrawingPage *pageVp, Gui::Document* doc, QWidget* parent)
  : Gui::MDIView(doc, parent), pageGui(pageVp)
{
    m_view = new CanvasView(pageVp);

    m_backgroundAction = new QAction(tr("&Background"), this);
    m_backgroundAction->setEnabled(false);
    m_backgroundAction->setCheckable(true);
    m_backgroundAction->setChecked(true);
    connect(m_backgroundAction, SIGNAL(toggled(bool)), m_view, SLOT(setViewBackground(bool)));

    m_exportSVGAction = new QAction(tr("&Export SVG"), this);
    connect(m_exportSVGAction, SIGNAL(triggered()), this, SLOT(saveSVG()));

    m_outlineAction = new QAction(tr("&Outline"), this);
    m_outlineAction->setEnabled(false);
    m_outlineAction->setCheckable(true);
    m_outlineAction->setChecked(false);
    connect(m_outlineAction, SIGNAL(toggled(bool)), m_view, SLOT(setViewOutline(bool)));    //in CanvasView

    m_nativeAction = new QAction(tr("&Native"), this);
    m_nativeAction->setCheckable(true);
    m_nativeAction->setChecked(false);
#ifndef QT_NO_OPENGL
    m_glAction = new QAction(tr("&OpenGL"), this);
    m_glAction->setCheckable(true);
#endif
    m_imageAction = new QAction(tr("&Image"), this);
    m_imageAction->setCheckable(true);

#ifndef QT_NO_OPENGL
    m_highQualityAntialiasingAction = new QAction(tr("&High Quality Antialiasing"), this);
    m_highQualityAntialiasingAction->setEnabled(false);
    m_highQualityAntialiasingAction->setCheckable(true);
    m_highQualityAntialiasingAction->setChecked(false);
    connect(m_highQualityAntialiasingAction, SIGNAL(toggled(bool)),
            m_view, SLOT(setHighQualityAntialiasing(bool)));
#endif

    isSlectionBlocked = false;

    QActionGroup *rendererGroup = new QActionGroup(this);
    rendererGroup->addAction(m_nativeAction);
#ifndef QT_NO_OPENGL
    rendererGroup->addAction(m_glAction);
#endif
    rendererGroup->addAction(m_imageAction);
    connect(rendererGroup, SIGNAL(triggered(QAction *)),
            this, SLOT(setRenderer(QAction *)));

    setWindowTitle(tr("dummy[*]"));      //Yuck. prevents "QWidget::setWindowModified: The window title does not contain a '[*]' placeholder"
    setCentralWidget(m_view);

    m_orientation = QPrinter::Landscape;
    m_pageSize = QPrinter::A4;

    // Connect Signals and Slots
    QObject::connect(
        m_view->scene(), SIGNAL(selectionChanged()),
        this           , SLOT  (selectionChanged())
       );


     // A fresh page is added and we iterate through its collected children and add these to Canvas View
     // if docobj is a featureviewcollection (ex orthogroup), add its child views. if there are ever children that have children,
     // we'll have to make this recursive. -WF
    const std::vector<App::DocumentObject*> &grp = pageGui->getPageObject()->Views.getValues();
    std::vector<App::DocumentObject*> childViews;
    for (std::vector<App::DocumentObject*>::const_iterator it = grp.begin();it != grp.end(); ++it) {
        attachView(*it);
        Drawing::FeatureViewCollection* collect = dynamic_cast<Drawing::FeatureViewCollection *>(*it);
        if (collect) {
            childViews = collect->Views.getValues();
            for (std::vector<App::DocumentObject*>::iterator itChild = childViews.begin();itChild != childViews.end(); ++itChild) {
                attachView(*itChild);
            }
        }
    }
    //when restoring, it is possible for a Dimension to be loaded before the ViewPart it applies to
    //therefore we need to make sure parentage of the graphics representation is set properly. bit of a kludge.
    setDimensionGroups();

    App::DocumentObject *obj = pageGui->getPageObject()->Template.getValue();
    if(obj && obj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId())) {
        Drawing::FeatureTemplate *pageTemplate = dynamic_cast<Drawing::FeatureTemplate *>(obj);
        attachTemplate(pageTemplate);
    }

}

DrawingView::~DrawingView()
{
  // Safely remove graphicview items that have built up TEMP SOLUTION
  for(QList<QGraphicsItemView*>::iterator it = deleteItems.begin(); it != deleteItems.end(); ++it) {
      (*it)->deleteLater();
  }
  deleteItems.clear();

  delete m_view;
}

void DrawingView::setDimensionGroups(void)
{
    const std::vector<QGraphicsItemView *> &allItems = m_view->getViews();
    std::vector<QGraphicsItemView *>::const_iterator itInspect;
    int dimItemType = QGraphicsItem::UserType + 106;

    for (itInspect = allItems.begin(); itInspect != allItems.end(); itInspect++) {
        if (((*itInspect)->type() == dimItemType) && (!(*itInspect)->group())) {
            QGraphicsItemView* parent = m_view->findParent((*itInspect));
            if (parent) {
                QGraphicsItemViewDimension* dim = dynamic_cast<QGraphicsItemViewDimension*>((*itInspect));
                m_view->addDimToParent(dim,parent);
            }
        }
    }
}
void DrawingView::findPrinterSettings(const QString& fileName)
{
    if (fileName.indexOf(QLatin1String("Portrait"), Qt::CaseInsensitive) >= 0) {
        m_orientation = QPrinter::Portrait;
    }
    else {
        m_orientation = QPrinter::Landscape;
    }

    QMap<QPrinter::PageSize, QString> pageSizes;
    pageSizes[QPrinter::A0] = QString::fromLatin1("A0");
    pageSizes[QPrinter::A1] = QString::fromLatin1("A1");
    pageSizes[QPrinter::A2] = QString::fromLatin1("A2");
    pageSizes[QPrinter::A3] = QString::fromLatin1("A3");
    pageSizes[QPrinter::A4] = QString::fromLatin1("A4");
    pageSizes[QPrinter::A5] = QString::fromLatin1("A5");
    pageSizes[QPrinter::A6] = QString::fromLatin1("A6");
    pageSizes[QPrinter::A7] = QString::fromLatin1("A7");
    pageSizes[QPrinter::A8] = QString::fromLatin1("A8");
    pageSizes[QPrinter::A9] = QString::fromLatin1("A9");
    pageSizes[QPrinter::B0] = QString::fromLatin1("B0");
    pageSizes[QPrinter::B1] = QString::fromLatin1("B1");
    pageSizes[QPrinter::B2] = QString::fromLatin1("B2");
    pageSizes[QPrinter::B3] = QString::fromLatin1("B3");
    pageSizes[QPrinter::B4] = QString::fromLatin1("B4");
    pageSizes[QPrinter::B5] = QString::fromLatin1("B5");
    pageSizes[QPrinter::B6] = QString::fromLatin1("B6");
    pageSizes[QPrinter::B7] = QString::fromLatin1("B7");
    pageSizes[QPrinter::B8] = QString::fromLatin1("B8");
    pageSizes[QPrinter::B9] = QString::fromLatin1("B9");
    pageSizes[QPrinter::Letter] = QString::fromLatin1("Letter");
    pageSizes[QPrinter::Legal] = QString::fromLatin1("Legal");
    for (QMap<QPrinter::PageSize, QString>::iterator it = pageSizes.begin(); it != pageSizes.end(); ++it) {
        if (fileName.startsWith(it.value(), Qt::CaseInsensitive)) {
            m_pageSize = it.key();
            break;
        }
    }
}

void DrawingView::setDocumentObject(const std::string& name)
{
    m_objectName = name;
}

void DrawingView::closeEvent(QCloseEvent* ev)
{
    MDIView::closeEvent(ev);
    if (!ev->isAccepted())
        return;

    // when closing the view from GUI notify the view provider to mark it invisible
    if (_pcDocument && !m_objectName.empty()) {
        App::Document* doc = _pcDocument->getDocument();
        if (doc) {
            App::DocumentObject* obj = doc->getObject(m_objectName.c_str());
            Gui::ViewProvider* vp = _pcDocument->getViewProvider(obj);
            if (vp)
                vp->hide();
        }
    }
}

void DrawingView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(m_backgroundAction);
    menu.addAction(m_outlineAction);
    menu.addAction(m_exportSVGAction);
    QMenu* submenu = menu.addMenu(tr("&Renderer"));
    submenu->addAction(m_nativeAction);
    submenu->addAction(m_glAction);
    submenu->addAction(m_imageAction);
    submenu->addSeparator();
    submenu->addAction(m_highQualityAntialiasingAction);
    menu.exec(event->globalPos());
}

void DrawingView::attachTemplate(Drawing::FeatureTemplate *obj)
{
    m_view->setPageTemplate(obj);
    double width  =  obj->Width.getValue();
    double height =  obj->Height.getValue();
    m_view->scene()->setSceneRect(QRectF(-1.,-height,width+1.,height));         //the +/- 1 is because of the way the template is define???
}

int DrawingView::attachView(App::DocumentObject *obj)
{
    QGraphicsItemView *qview = 0;
    if(obj->getTypeId().isDerivedFrom(Drawing::FeatureViewSection::getClassTypeId()) ) {
        Drawing::FeatureViewSection *viewSect = dynamic_cast<Drawing::FeatureViewSection *>(obj);
        qview = m_view->addViewSection(viewSect);
    } else if (obj->getTypeId().isDerivedFrom(Drawing::FeatureViewPart::getClassTypeId()) ) {
        Drawing::FeatureViewPart *viewPart = dynamic_cast<Drawing::FeatureViewPart *>(obj);
        qview = m_view->addViewPart(viewPart);
    } else if (obj->getTypeId().isDerivedFrom(Drawing::FeatureProjGroup::getClassTypeId()) ) {
        Drawing::FeatureProjGroup *view = dynamic_cast<Drawing::FeatureProjGroup *>(obj);
        qview = m_view->addProjectionGroup(view);
    } else if (obj->getTypeId().isDerivedFrom(Drawing::FeatureViewCollection::getClassTypeId()) ) {
        Drawing::FeatureViewCollection *collection = dynamic_cast<Drawing::FeatureViewCollection *>(obj);
        qview =  m_view->addFeatureView(collection);
    } else if(obj->getTypeId().isDerivedFrom(Drawing::FeatureViewDimension::getClassTypeId()) ) {
        Drawing::FeatureViewDimension *viewDim = dynamic_cast<Drawing::FeatureViewDimension *>(obj);
        qview = m_view->addViewDimension(viewDim);
    } else if(obj->getTypeId().isDerivedFrom(Drawing::FeatureViewAnnotation::getClassTypeId()) ) {
        Drawing::FeatureViewAnnotation *viewDim = dynamic_cast<Drawing::FeatureViewAnnotation *>(obj);
        qview = m_view->addFeatureViewAnnotation(viewDim);
    } else if(obj->getTypeId().isDerivedFrom(Drawing::FeatureViewSymbol::getClassTypeId()) ) {
        Drawing::FeatureViewSymbol *viewSym = dynamic_cast<Drawing::FeatureViewSymbol *>(obj);
        qview = m_view->addFeatureViewSymbol(viewSym);
    } else if(obj->getTypeId().isDerivedFrom(Drawing::FeatureViewClip::getClassTypeId()) ) {
        Drawing::FeatureViewClip *viewClip = dynamic_cast<Drawing::FeatureViewClip *>(obj);
        qview = m_view->addFeatureViewClip(viewClip);
    }

    if(!qview)
        return -1;
    else
        return m_view->getViews().size();
}

void DrawingView::preSelectionChanged(const QPoint &pos)
{
    QObject *obj = QObject::sender();

    if(!obj)
        return;

    // Check if an edge was preselected
    QGraphicsItemEdge *edge   = dynamic_cast<QGraphicsItemEdge *>(obj);
    QGraphicsItemVertex *vert = dynamic_cast<QGraphicsItemVertex *>(obj);
    if(edge) {

        // Find the parent view that this edges is contained within
        QGraphicsItem *parent = edge->parentItem();
        if(!parent)
            return;

        QGraphicsItemView *viewItem = dynamic_cast<QGraphicsItemView *>(parent);
        if(!viewItem)
          return;

        Drawing::FeatureView *viewObj = viewItem->getViewObject();
        std::stringstream ss;
        //ss << "Edge" << edge->getReference();
        ss << "Edge" << edge->getProjIndex();
        //bool accepted =
        static_cast<void> (Gui::Selection().setPreselect(viewObj->getDocument()->getName()
                                     ,viewObj->getNameInDocument()
                                     ,ss.str().c_str()
                                     ,pos.x()
                                     ,pos.y()
                                     ,0));

    } else if(vert) {
              // Find the parent view that this edges is contained within
              //WF: sb Vertex??
        QGraphicsItem *parent = vert->parentItem();
        if(!parent)
            return;

        QGraphicsItemView *viewItem = dynamic_cast<QGraphicsItemView *>(parent);
        if(!viewItem)
          return;

        Drawing::FeatureView *viewObj = viewItem->getViewObject();
        std::stringstream ss;
        ss << "Edge" << vert->getReference();   //WF: sb Vertex??
        //bool accepted =
        static_cast<void> (Gui::Selection().setPreselect(viewObj->getDocument()->getName()
                                     ,viewObj->getNameInDocument()
                                     ,ss.str().c_str()
                                     ,pos.x()
                                     ,pos.y()
                                     ,0));
    } else {
            // Check if an edge was preselected
            //WF: sb View?
        QGraphicsItemView *view = qobject_cast<QGraphicsItemView *>(obj);

        if(!view)
            return;
        Drawing::FeatureView *viewObj = view->getViewObject();
        Gui::Selection().setPreselect(viewObj->getDocument()->getName()
                                     ,viewObj->getNameInDocument()
                                     ,""
                                     ,pos.x()
                                     ,pos.y()
                                     ,0);
    }
}

void DrawingView::blockSelection(const bool state)
{
  isSlectionBlocked = state;
}

void DrawingView::clearSelection()
{
  blockSelection(true);
  std::vector<QGraphicsItemView *> views = m_view->getViews();

  // Iterate through all views and unselect all
  for (std::vector<QGraphicsItemView *>::iterator it = views.begin(); it != views.end(); ++it) {
      QGraphicsItemView *item = *it;
      item->setSelected(false);
      item->updateView();
  }

  blockSelection(false);
}

void DrawingView::selectFeature(App::DocumentObject *obj, const bool isSelected)
{
    // Update CanvasView's selection based on Selection made outside Drawing Interace
  QGraphicsItemView *view = m_view->findView(obj);

  blockSelection(true);
  if(view) {
      view->setSelected(isSelected);
      view->updateView();
  }
  blockSelection(false);
}

void DrawingView::selectionChanged()
{
    if(isSlectionBlocked)
      return;

    QList<QGraphicsItem *> selection = m_view->scene()->selectedItems();

    bool block = blockConnection(true); // avoid to be notified by itself

    Gui::Selection().clearSelection();
    for (QList<QGraphicsItem *>::iterator it = selection.begin(); it != selection.end(); ++it) {
        // All selectable items must be of QGraphicsItemView type

        QGraphicsItemView *itemView = dynamic_cast<QGraphicsItemView *>(*it);
        if(itemView == 0) {
            QGraphicsItemEdge *edge = dynamic_cast<QGraphicsItemEdge *>(*it);
            if(edge) {

                // Find the parent view that this edges is contained within
                QGraphicsItem *parent = edge->parentItem();
                if(!parent)
                    return;

                QGraphicsItemView *viewItem = dynamic_cast<QGraphicsItemView *>(parent);
                if(!viewItem)
                  return;

                Drawing::FeatureView *viewObj = viewItem->getViewObject();

                std::stringstream ss;
                //ss << "Edge" << edge->getReference();
                ss << "Edge" << edge->getProjIndex();
                //bool accepted =
                static_cast<void> (Gui::Selection().addSelection(viewObj->getDocument()->getName(),
                                              viewObj->getNameInDocument(),
                                              ss.str().c_str()));
            }

            QGraphicsItemVertex *vert = dynamic_cast<QGraphicsItemVertex *>(*it);
            if(vert) {
              // Find the parent view that this edges is contained within
              //WF: sb Vertex
                QGraphicsItem *parent = vert->parentItem();
                if(!parent)
                    return;

                QGraphicsItemView *viewItem = dynamic_cast<QGraphicsItemView *>(parent);
                if(!viewItem)
                  return;

                Drawing::FeatureView *viewObj = viewItem->getViewObject();

                std::stringstream ss;
                ss << "Vertex" << vert->getProjIndex();
                //bool accepted =
                static_cast<void> (Gui::Selection().addSelection(viewObj->getDocument()->getName(),
                                              viewObj->getNameInDocument(),
                                              ss.str().c_str()));

            }

            QGraphicsItemDatumLabel *dimLabel = dynamic_cast<QGraphicsItemDatumLabel*>(*it);
            if(dimLabel) {
                // Find the parent view (dimLabel->dim->view)

                QGraphicsItem *dimParent = dimLabel->parentItem();

                if(!dimParent)
                    return;

                QGraphicsItemView *dimItem = dynamic_cast<QGraphicsItemView *>(dimParent);

                if(!dimItem)
                  return;

                Drawing::FeatureView *dimObj = dimItem->getViewObject();

                //bool accepted =
                static_cast<void> (Gui::Selection().addSelection(dimObj->getDocument()->getName(),dimObj->getNameInDocument()));

            }
            continue;

        } else {

            Drawing::FeatureView *viewObj = itemView->getViewObject();

            std::string doc_name = viewObj->getDocument()->getName();
            std::string obj_name = viewObj->getNameInDocument();

            Gui::Selection().addSelection(doc_name.c_str(), obj_name.c_str());
        }

    }

    blockConnection(block);
}

void DrawingView::updateTemplate(bool forceUpdate)
{
    App::DocumentObject *templObj = pageGui->getPageObject()->Template.getValue();
    // TODO: what if template has been deleted? templObj will be NULL. segfault?
    if (!templObj) {
        Base::Console().Log("INFO - DrawingView::updateTemplate - Page: %s has NO template!!\n",pageGui->getPageObject()->getNameInDocument());
        return;
    }

    if(pageGui->getPageObject()->Template.isTouched() || templObj->isTouched()) {
        // Template is touched so update

        if(forceUpdate ||
           (templObj && templObj->isTouched() && templObj->isDerivedFrom(Drawing::FeatureTemplate::getClassTypeId())) ) {

            QGraphicsItemTemplate *qItemTemplate = m_view->getTemplate();

            if(qItemTemplate) {
                Drawing::FeatureTemplate *pageTemplate = dynamic_cast<Drawing::FeatureTemplate *>(templObj);
                qItemTemplate->setTemplate(pageTemplate);
                qItemTemplate->updateView();
            }
        }
    }

    // m_view->setPageFeature(pageFeature); redundant
}

void DrawingView::updateDrawing(bool forceUpdate)
{
    // We cannot guarantee if the number of graphical representations (QGIVxxxx) have changed so check the number
    // Why?
    const std::vector<QGraphicsItemView *> &graphicsList = m_view->getViews();
    const std::vector<App::DocumentObject*> &pageChildren  = pageGui->getPageObject()->Views.getValues();

    // Count total # DocumentObjects in Page
    unsigned int docObjCount = 0;
    for(std::vector<App::DocumentObject*>::const_iterator it = pageChildren.begin(); it != pageChildren.end(); ++it) {
        App::DocumentObject *docObj = *it;
        if(docObj->getTypeId().isDerivedFrom(Drawing::FeatureViewCollection::getClassTypeId())) {
            Drawing::FeatureViewCollection *collection = dynamic_cast<Drawing::FeatureViewCollection *>(docObj);
            docObjCount += collection->countChildren(); // Include self
        }
        docObjCount += 1;
    }

    if(graphicsList.size() < docObjCount) {
        // there are more DocumentObjects than graphical representations (QGIVxxxx's)
        // Find which DocumentObjects have no graphical representation (QGIVxxxx)
        // Iterate over DocumentObjects without graphical representations and create the QGIVxxxx
        // TODO think of a better algorithm to deal with any changes to views list
        std::vector<App::DocumentObject*> notFnd;
        findMissingViews(pageChildren, notFnd);
        for(std::vector<App::DocumentObject*>::const_iterator it = notFnd.begin(); it != notFnd.end(); ++it) {
            attachView(*it);
        }
    } else if(graphicsList.size() > docObjCount) {
        // There are more graphical representations (QGIVxxxx) than DocumentObjects
        // Remove the orphans
        std::vector<QGraphicsItemView *>::const_iterator itGraphics = graphicsList.begin();
        std::vector<QGraphicsItemView *> newGraphicsList;
        bool fnd = false;
        while(itGraphics != graphicsList.end()) {
            fnd = orphanExists((*itGraphics)->getViewName(), pageChildren);
            if(fnd) {
                newGraphicsList.push_back(*itGraphics);
            } else {
                if (m_view->scene() == (*itGraphics)->scene()) {
                    (*itGraphics)->hide();
                    m_view->scene()->removeItem(*itGraphics);
                } else {   // this "shouldn't" happen, but it does
                    Base::Console().Log("ERROR - DrawingView::updateDrawing - %s already removed from QGraphicsScene\n",
                                        (*itGraphics)->getViewName());
                }
                deleteItems.append(*itGraphics); // delete in the destructor when completly safe. TEMP SOLUTION
            }
            itGraphics++;
        }

        // Update the CanvasView (QGraphicsView) list of QGIVxxxx
        m_view->setViews(newGraphicsList);
    }

    // Update all the QGIVxxxx
    const std::vector<QGraphicsItemView *> &upviews = m_view->getViews();
    for(std::vector<QGraphicsItemView *>::const_iterator it = upviews.begin(); it != upviews.end(); ++it) {
        if((*it)->getViewObject()->isTouched() ||
           forceUpdate) {
            (*it)->updateView(forceUpdate);
        }
    }
}

void DrawingView::findMissingViews(const std::vector<App::DocumentObject*> &list, std::vector<App::DocumentObject*> &missing)
{
    for(std::vector<App::DocumentObject*>::const_iterator it = list.begin(); it != list.end(); ++it) {

        if(!hasQView(*it))
             missing.push_back(*it);

        if((*it)->getTypeId().isDerivedFrom(Drawing::FeatureViewCollection::getClassTypeId())) {
            std::vector<App::DocumentObject*> missingChildViews;
            Drawing::FeatureViewCollection *collection = dynamic_cast<Drawing::FeatureViewCollection *>(*it);
            // Find Child Views recursively
            findMissingViews(collection->Views.getValues(), missingChildViews);

            // Append the views to current missing list
            for(std::vector<App::DocumentObject*>::const_iterator it = missingChildViews.begin(); it != missingChildViews.end(); ++it) {
                missing.push_back(*it);
            }
        }
    }
}

/// Helper function
bool DrawingView::hasQView(App::DocumentObject *obj)
{
    const std::vector<QGraphicsItemView *> &views = m_view->getViews();
    std::vector<QGraphicsItemView *>::const_iterator qview = views.begin();

    while(qview != views.end()) {
        // Unsure if we can compare pointers so rely on name
        if(strcmp((*qview)->getViewObject()->getNameInDocument(), obj->getNameInDocument()) == 0) {
            return true;
        }
        qview++;
    }

    return false;
}

/// Helper function
bool DrawingView::orphanExists(const char *viewName, const std::vector<App::DocumentObject*> &list)
{
    for(std::vector<App::DocumentObject*>::const_iterator it = list.begin(); it != list.end(); ++it) {

        //Check child objects too recursively
        if((*it)->isDerivedFrom(Drawing::FeatureViewCollection::getClassTypeId())) {
            Drawing::FeatureViewCollection *collection = dynamic_cast<Drawing::FeatureViewCollection *>(*it);
            if(orphanExists(viewName, collection->Views.getValues()))
                return true;
        }

        // Unsure if we can compare pointers so rely on name
        if(strcmp(viewName, (*it)->getNameInDocument()) == 0) {
            return true;
        }
    }
    return false;
}


void DrawingView::setRenderer(QAction *action)
{
#ifndef QT_NO_OPENGL
    m_highQualityAntialiasingAction->setEnabled(false);
#endif

    if (action == m_nativeAction)
        m_view->setRenderer(CanvasView::Native);
#ifndef QT_NO_OPENGL
    else if (action == m_glAction) {
        m_highQualityAntialiasingAction->setEnabled(true);
        m_view->setRenderer(CanvasView::OpenGL);
    }
#endif
    else if (action == m_imageAction) {
        m_view->setRenderer(CanvasView::Image);
    }
}
bool DrawingView::onMsg(const char* pMsg, const char** ppReturn)
{
    if (strcmp("ViewFit",pMsg) == 0) {
        viewAll();
        return true;
    } else if (strcmp("Redo", pMsg) == 0 ) {
        getAppDocument()->redo();
        Gui::Command::updateActive();
        return true;
    } else if (strcmp("Save", pMsg) == 0 ) {
        getAppDocument()->save();
        Gui::Command::updateActive();
        return true;
    } else if (strcmp("SaveAs", pMsg) == 0 ) {
        getAppDocument()->saveAs("");
        Gui::Command::updateActive();
        return true;
    } else if (strcmp("Undo", pMsg) == 0 ) {
        getAppDocument()->undo();
        Gui::Command::updateActive();
        return true;
    }
    else  if(strcmp("Undo",pMsg) == 0 ) {
        Gui::Document *doc = getGuiDocument();
        if (doc) {
            doc->undo(1);
            return true;
        }
    }
    else  if(strcmp("Redo",pMsg) == 0 ) {
        Gui::Document *doc = getGuiDocument();
        if (doc) {
            doc->redo(1);
            return true;
        }
    }
    return false;
}

bool DrawingView::onHasMsg(const char* pMsg) const
{
    if (strcmp("ViewFit",pMsg) == 0)
        return true;
    else if(strcmp("Redo", pMsg) == 0 && getAppDocument()->getAvailableRedos() > 0)
        return true;
    else if(strcmp("Undo", pMsg) == 0 && getAppDocument()->getAvailableUndos() > 0)
        return true;
    else if (strcmp("Print",pMsg) == 0)
        return true;
    else if (strcmp("Save",pMsg) == 0)
        return true;
    else if (strcmp("SaveAs",pMsg) == 0)
        return true;
    else if (strcmp("PrintPreview",pMsg) == 0)
        return true;
    else if (strcmp("PrintPdf",pMsg) == 0)
        return true;
    return false;
}

void DrawingView::onRelabel(Gui::Document *pDoc)
{
    if (!bIsPassive && pDoc) {
        QString cap = QString::fromLatin1("%1 : %2[*]")
            .arg(QString::fromUtf8(pDoc->getDocument()->Label.getValue()))
            .arg(objectName());
        setWindowTitle(cap);
    }
}

void DrawingView::printPdf()
{
    Gui::FileOptionsDialog dlg(this, 0);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setWindowTitle(tr("Export PDF"));
    dlg.setFilters(QStringList() << QString::fromLatin1("%1 (*.pdf)").arg(tr("PDF file")));

    QGridLayout *gridLayout;
    QGridLayout *formLayout;
    QGroupBox *groupBox;
    QListWidget *listWidget;
    QListWidgetItem* item;
    QWidget *form = new QWidget(&dlg);
    form->resize(40, 300);
    formLayout = new QGridLayout(form);
    groupBox = new QGroupBox(form);
    gridLayout = new QGridLayout(groupBox);
    listWidget = new QListWidget(groupBox);
    gridLayout->addWidget(listWidget, 0, 0, 1, 1);
    formLayout->addWidget(groupBox, 0, 0, 1, 1);

    groupBox->setTitle(tr("Page sizes"));
    item = new QListWidgetItem(tr("A0"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::A0));
    item = new QListWidgetItem(tr("A1"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::A1));
    item = new QListWidgetItem(tr("A2"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::A2));
    item = new QListWidgetItem(tr("A3"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::A3));
    item = new QListWidgetItem(tr("A4"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::A4));
    item = new QListWidgetItem(tr("A5"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::A5));
    item = new QListWidgetItem(tr("Letter"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::Letter));
    item = new QListWidgetItem(tr("Legal"), listWidget);
    item->setData(Qt::UserRole, QVariant(QPrinter::Legal));
    //listWidget->item(4)->setSelected(true); // by default A4
    int index = 4; // by default A4
    for (int i=0; i<listWidget->count(); i++) {
        if (listWidget->item(i)->data(Qt::UserRole).toInt() == m_pageSize) {
            index = i;
            break;
        }
    }
    listWidget->item(index)->setSelected(true);
    dlg.setOptionsWidget(Gui::FileOptionsDialog::ExtensionRight, form, false);

    if (dlg.exec() == QDialog::Accepted) {
        Gui::WaitCursor wc;
        QString filename = dlg.selectedFiles().front();
        QPrinter printer(QPrinter::HighResolution);
        printer.setFullPage(true);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filename);
        printer.setOrientation(m_orientation);
        QList<QListWidgetItem*> items = listWidget->selectedItems();
        if (items.size() == 1) {
            int AX = items.front()->data(Qt::UserRole).toInt();
            printer.setPaperSize(QPrinter::PageSize(AX));
        }

        print(&printer);
    }
}

void DrawingView::print()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    printer.setPageSize(m_pageSize);
    printer.setOrientation(m_orientation);
    QPrintDialog dlg(&printer, this);
    if (dlg.exec() == QDialog::Accepted) {
        print(&printer);
    }
}

void DrawingView::printPreview()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    printer.setPageSize(m_pageSize);
    printer.setOrientation(m_orientation);

    QPrintPreviewDialog dlg(&printer, this);
    connect(&dlg, SIGNAL(paintRequested (QPrinter *)),
            this, SLOT(print(QPrinter *)));
    dlg.exec();
}


void DrawingView::print(QPrinter* printer)
{
    // As size of the render area paperRect() should be used. When performing a real
    // print pageRect() may also work but the output is cropped at the bottom part.
    // So, independent whether pageRect() or paperRect() is used there is no scaling effect.
    // However, when using a different paper size as set in the drawing template (e.g.
    // DIN A5 instead of DIN A4) then the output is scaled.
    //
    // When creating a PDF file there seems to be no difference between pageRect() and
    // paperRect().
    //
    // When showing the preview of a print paperRect() must be used because with pageRect()
    // a certain scaling effect can be observed and the content becomes smaller.
    QPaintEngine::Type paintType = printer->paintEngine()->type();
    if (printer->outputFormat() == QPrinter::NativeFormat) {
        int w = printer->widthMM();
        int h = printer->heightMM();
        QPrinter::PaperSize realPaperSize = getPageSize(w, h);
        QPrinter::PaperSize curPaperSize = printer->paperSize();

        // for the preview a 'Picture' paint engine is used which we don't
        // care if it uses wrong printer settings
        bool doPrint = paintType != QPaintEngine::Picture;

        if (doPrint && printer->orientation() != m_orientation) {
            int ret = QMessageBox::warning(this, tr("Different orientation"),
                tr("The printer uses a different orientation  than the drawing.\n"
                   "Do you want to continue?"),
                   QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes)
                return;
        }
        else if (doPrint && realPaperSize != m_pageSize) {
            int ret = QMessageBox::warning(this, tr("Different paper size"),
                tr("The printer uses a different paper size than the drawing.\n"
                   "Do you want to continue?"),
                   QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes)
                return;
        }
        else if (doPrint && curPaperSize != m_pageSize) {
            int ret = QMessageBox::warning(this, tr("Different paper size"),
                tr("The printer uses a different paper size than the drawing.\n"
                   "Do you want to continue?"),
                   QMessageBox::Yes | QMessageBox::No);
            if (ret != QMessageBox::Yes)
                return;
        }
    }

    QPainter p(printer);
    if (!p.isActive() && !printer->outputFileName().isEmpty()) {
        qApp->setOverrideCursor(Qt::ArrowCursor);
        QMessageBox::critical(this, tr("Opening file failed"),
            tr("Can't open file '%1' for writing.").arg(printer->outputFileName()));
        qApp->restoreOverrideCursor();
        return;
    } 
    QRect rect = printer->paperRect();
#ifdef Q_OS_WIN32
    // On Windows the preview looks broken when using paperRect as render area.
    // Although the picture is scaled when using pageRect, it looks just fine.
    if (paintType == QPaintEngine::Picture)
        rect = printer->pageRect();
#endif

    //bool block =
    static_cast<void> (blockConnection(true)); // avoid to be notified by itself
    Gui::Selection().clearSelection();

    m_view->toggleEdit(false);
    m_view->scene()->update();

    Gui::Selection().clearSelection();

    m_view->scene()->render(&p, rect);

    // Reset
    m_view->toggleEdit(true);
}

//QPrinter::PageSize is obsolete. Use QPrinter::PaperSize instead.
QPrinter::PageSize DrawingView::getPageSize(int w, int h) const
{
    static const float paperSizes[][2] = {
        {210, 297}, // A4
        {176, 250}, // B5
        {215.9f, 279.4f}, // Letter
        {215.9f, 355.6f}, // Legal
        {190.5f, 254}, // Executive
        {841, 1189}, // A0
        {594, 841}, // A1
        {420, 594}, // A2
        {297, 420}, // A3
        {148, 210}, // A5
        {105, 148}, // A6
        {74, 105}, // A7
        {52, 74}, // A8
        {37, 52}, // A8
        {1000, 1414}, // B0
        {707, 1000}, // B1
        {31, 44}, // B10
        {500, 707}, // B2
        {353, 500}, // B3
        {250, 353}, // B4
        {125, 176}, // B6
        {88, 125}, // B7
        {62, 88}, // B8
        {33, 62}, // B9
        {163, 229}, // C5E
        {105, 241}, // US Common
        {110, 220}, // DLE
        {210, 330}, // Folio
        {431.8f, 279.4f}, // Ledger
        {279.4f, 431.8f} // Tabloid
    };

    QPrinter::PageSize ps = QPrinter::Custom;
    for (int i=0; i<30; i++) {
        if (std::abs(paperSizes[i][0]-w) <= 1 &&
            std::abs(paperSizes[i][1]-h) <= 1) {
            ps = static_cast<QPrinter::PageSize>(i);
            break;
        }
        else
        if (std::abs(paperSizes[i][0]-h) <= 1 &&
            std::abs(paperSizes[i][1]-w) <= 1) {
            ps = static_cast<QPrinter::PageSize>(i);
            break;
        }
    }

    return ps;
}

void DrawingView::viewAll()
{
    m_view->fitInView(m_view->scene()->sceneRect(), Qt::KeepAspectRatio);
}

void DrawingView::onSelectionChanged(const Gui::SelectionChanges& msg)
{
    if (msg.Type == Gui::SelectionChanges::ClrSelection) {

    }
    else if (msg.Type == Gui::SelectionChanges::AddSelection ||
             msg.Type == Gui::SelectionChanges::RmvSelection) {
        //bool select = (msg.Type == Gui::SelectionChanges::AddSelection);
        // Check if it is a view object
    }
    else if (msg.Type == Gui::SelectionChanges::SetSelection) {
        // do nothing here
    }
}

void DrawingView::saveSVG()
{
    QStringList filter;
    filter << QObject::tr("SVG (*.svg)");
    filter << QObject::tr("All Files (*.*)");
    QString fn = Gui::FileDialog::getSaveFileName(Gui::getMainWindow(), QObject::tr("Export page as SVG"),
                                                  QString(), filter.join(QLatin1String(";;")));
    if (fn.isEmpty()) {
      return;
    }

    Drawing::FeaturePage *page = pageGui->getPageObject();
    const QString docName = QString::fromUtf8(page->getDocument()->getName());
    const QString pageName = QString::fromUtf8(page->getNameInDocument());
    QString svgDescription = tr("Drawing page: ") +
                             pageName +
                             tr(" exported from FreeCAD document: ") +
                             docName;

    //int width  =  page->getPageWidth();
    //int height =  page->getPageHeight();
    //Base::Console().Message("TRACE - saveSVG - page width: %d height: %d\n",width,height);    //A4 297x210
    QSvgGenerator svgGen;
    svgGen.setFileName(fn);
    svgGen.setSize(QSize((int) page->getPageWidth(), (int)page->getPageHeight()));
    svgGen.setViewBox(QRect(0, 0, page->getPageWidth(), page->getPageHeight()));
    //TODO: Exported Svg file is not quite right. <svg width="301.752mm" height="213.36mm" viewBox="0 0 297 210"... A4: 297x210
    //      Page too small (A4 vs Letter? margins?)
    //TODO: text in Qt is in mm (actually scene units).  text in SVG is points(?). fontsize in export file is too small by 1/2.835.
    //      resize all textItem before export?
    //      postprocess generated file to mult all font-size attrib by 2.835 to get pts?
    //      duplicate all textItems and only show the appropriate one for screen/print vs export?
    svgGen.setResolution(25.4000508);    // mm/inch??  docs say this is DPI
    //svgGen.setResolution(600);    // resulting page is ~12.5x9mm
    //svgGen.setResolution(96);     // page is ~78x55mm
    svgGen.setTitle(QObject::tr("FreeCAD SVG Export"));
    svgGen.setDescription(svgDescription);

    //bool block =
    static_cast<void> (blockConnection(true)); // avoid to be notified by itself
    Gui::Selection().clearSelection();

    m_view->toggleEdit(false);             //fiddle cache, cosmetic lines, vertices, etc
    m_view->scene()->update();

    Gui::Selection().clearSelection();
    QPainter p;

    p.begin(&svgGen);
    m_view->scene()->render(&p);
    p.end();

    m_view->toggleEdit(true);
}

PyObject* DrawingView::getPyObject()
{
    Py_Return;
}

#include "moc_DrawingView.cpp"

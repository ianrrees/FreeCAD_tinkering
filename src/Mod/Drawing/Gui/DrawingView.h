/***************************************************************************
 *   Copyright (c) 2007 Jürgen Riegel <juergen.riegel@web.de>              *
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


#ifndef DRAWINGGUI_DRAWINGVIEW_H
#define DRAWINGGUI_DRAWINGVIEW_H

#include <Gui/MDIView.h>
#include <Gui/Selection.h>

#include <QGraphicsView>
#include <QPrinter>

QT_BEGIN_NAMESPACE
class QSlider;
class QAction;
class QActionGroup;
class QFile;
class QPopupMenu;
class QToolBar;
class QSvgWidget;
class QScrollArea;
class QPrinter;
QT_END_NAMESPACE
namespace Drawing {
class FeaturePage;
class FeatureTemplate;
}

namespace DrawingGui
{

class ViewProviderDrawingPage;
class CanvasView;
class QGraphicsItemView;

class DrawingGuiExport DrawingView : public Gui::MDIView, public Gui::SelectionObserver
{
    Q_OBJECT

public:
    DrawingView(ViewProviderDrawingPage *page, Gui::Document* doc, QWidget* parent = 0);
    virtual ~DrawingView();

public Q_SLOTS:
    //void load(const QString &path = QString());
    void setRenderer(QAction *action);
    void viewAll();
    void attachTemplate(Drawing::FeatureTemplate *obj);
    void saveSVG(); // TEMPORARY
    void selectionChanged();
    void preSelectionChanged(const QPoint &pos);
    void updateDrawing();
    void updateTemplate(bool force = false);

public:
   /// Observer message from the Selection
    void onSelectionChanged(const Gui::SelectionChanges& msg);

    bool onMsg(const char* pMsg,const char** ppReturn);
    bool onHasMsg(const char* pMsg) const;
    void onRelabel(Gui::Document *pDoc);
    void print();
    void printPdf();
    void printPreview();

    void selectFeature(App::DocumentObject *obj, bool state);
    void clearSelection();

    void blockSelection(bool isBlocked);

    void print(QPrinter* printer);
    void setDocumentObject(const std::string&);
    PyObject* getPyObject();

protected:
    void findMissingViews( const std::vector<App::DocumentObject*> &list, std::vector<App::DocumentObject*> &missing);
    bool hasQView(App::DocumentObject *obj);
    bool orphanExists(const char *viewName, const std::vector<App::DocumentObject*> &list);
    int attachView(App::DocumentObject *obj);
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent*);
    void findPrinterSettings(const QString&);
    QPrinter::PageSize getPageSize(int w, int h) const;

private:
    QAction *m_nativeAction;
    QAction *m_glAction;
    QAction *m_exportSVGAction;
    QAction *m_imageAction;
    QAction *m_highQualityAntialiasingAction;
    QAction *m_backgroundAction;
    QAction *m_outlineAction;

    std::string m_objectName;
    bool isSlectionBlocked;
    CanvasView *m_view;

    QString m_currentPath;
    QPrinter::Orientation m_orientation;
    QPrinter::PageSize m_pageSize;
    ViewProviderDrawingPage *pageGui;

    QList<QGraphicsItemView *> deleteItems;
};


} // namespace DrawingViewGui

#endif // DRAWINGGUI_DRAWINGVIEW_H

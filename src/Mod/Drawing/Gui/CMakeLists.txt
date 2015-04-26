if(MSVC)
    add_definitions(-DHAVE_ATANH -DHAVE_ASINH -DHAVE_ACOSH)
else(MSVC)
    add_definitions(-DHAVE_LIMITS_H -DHAVE_CONFIG_H)
endif(MSVC)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${COIN3D_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
    ${QT_INCLUDE_DIR}
    ${QT_USE_QTXML}
    ${OCC_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${XercesC_INCLUDE_DIRS}
    ${SOQT_INCLUDE_DIR}
)
link_directories(${OCC_LIBRARY_DIR})

set(DrawingGui_LIBS
    Drawing
    FreeCADGui
)


set(DrawingGui_MOC_HDRS
    DrawingView.h
    TaskDialog.h
    CanvasView.h
    QGraphicsItemTemplate.h
    QGraphicsItemSVGTemplate.h
    QGraphicsItemDrawingTemplate.h
    QGraphicsItemView.h
    QGraphicsItemViewCollection.h
    QGraphicsItemViewDimension.h
    QGraphicsItemViewOrthographic.h
    QGraphicsItemViewPart.h
    QGraphicsItemViewSection.h
    QGraphicsItemViewAnnotation.h
    QGraphicsItemViewSymbol.h
    TaskOrthographicViews.h
    TemplateTextField.h
)

fc_wrap_cpp(DrawingGui_MOC_SRCS ${DrawingGui_MOC_HDRS})
SOURCE_GROUP("Moc" FILES ${DrawingGui_MOC_SRCS})

qt4_add_resources(DrawingGui_SRCS Resources/Drawing.qrc)

set(DrawingGui_UIC_SRCS
    TaskOrthographicViews.ui
)

qt4_wrap_ui(DrawingGui_UIC_HDRS ${DrawingGui_UIC_SRCS})

SET(DrawingGui_SRCS
    ${DrawingGui_SRCS}
    AppDrawingGui.cpp
    AppDrawingGuiPy.cpp
    Command.cpp
    CommandCreateDims.cpp
    Resources/Drawing.qrc
    PreCompiled.cpp
    PreCompiled.h
    TaskDialog.cpp
    TaskDialog.h
    Workbench.cpp
    Workbench.h
    TaskOrthographicViews.ui
    TaskOrthographicViews.cpp
    TaskOrthographicViews.h
)
SET(DrawingGuiView_SRCS
    DrawingView.cpp
    DrawingView.h
    CanvasView.cpp
    CanvasView.h
    QGraphicsItemView.cpp
    QGraphicsItemView.h
    QGraphicsItemArrow.cpp
    QGraphicsItemArrow.h
    QGraphicsItemEdge.cpp
    QGraphicsItemEdge.h
    QGraphicsItemFace.cpp
    QGraphicsItemFace.h
    QGraphicsItemSVGTemplate.cpp
    QGraphicsItemSVGTemplate.h
    QGraphicsItemVertex.cpp
    QGraphicsItemVertex.h
    QGraphicsItemDrawingTemplate.cpp
    QGraphicsItemDrawingTemplate.h
    QGraphicsItemTemplate.cpp
    QGraphicsItemTemplate.h
    QGraphicsItemViewCollection.cpp
    QGraphicsItemViewCollection.h
    QGraphicsItemViewDimension.cpp
    QGraphicsItemViewDimension.h
    QGraphicsItemViewOrthographic.cpp
    QGraphicsItemViewOrthographic.h
    QGraphicsItemViewPart.cpp
    QGraphicsItemViewPart.h
    QGraphicsItemViewSection.cpp
    QGraphicsItemViewSection.h
    QGraphicsItemViewAnnotation.cpp
    QGraphicsItemViewAnnotation.h
    QGraphicsItemViewSymbol.cpp
    QGraphicsItemViewSymbol.h
    TemplateTextField.cpp
    TemplateTextField.h
)
SET(DrawingGuiViewProvider_SRCS
    ViewProviderPage.cpp
    ViewProviderPage.h
    ViewProviderView.cpp
    ViewProviderView.h
    ViewProviderOrthoView.cpp
    ViewProviderOrthoView.h
    ViewProviderTemplate.cpp
    ViewProviderTemplate.h
    ViewProviderDimension.cpp
    ViewProviderDimension.h
    ViewProviderViewPart.cpp
    ViewProviderViewPart.h
    ViewProviderViewOrthographic.cpp
    ViewProviderViewOrthographic.h
    ViewProviderViewSection.cpp
    ViewProviderViewSection.h
    ViewProviderAnnotation.cpp
    ViewProviderAnnotation.h
    ViewProviderSymbol.cpp
    ViewProviderSymbol.h
    ViewProviderClip.cpp
    ViewProviderClip.h
)

SOURCE_GROUP("Mod" FILES ${DrawingGui_SRCS})
SOURCE_GROUP("SVG-View" FILES ${DrawingGuiView_SRCS})
SOURCE_GROUP("ViewProvider" FILES ${DrawingGuiViewProvider_SRCS})

SET(DrawingGuiTaskDlgs_SRCS
    TaskOrthographicViews.ui
)
SOURCE_GROUP("TaskDialogs" FILES ${DrawingGuiTaskDlgs_SRCS})

if(MSVC)
    #add_definitions(-D_PreComp_)
    #GET_MSVC_PRECOMPILED_SOURCE("PreCompiled.cpp" PCH_SRCS ${DrawingGui_SRCS} ${DrawingGuiView_SRCS} ${DrawingGuiViewProvider_SRCS})
    #ADD_MSVC_PRECOMPILED_HEADER(DrawingGui PreCompiled.h PreCompiled.cpp PCH_SRCS)
endif(MSVC)

add_library(DrawingGui SHARED ${DrawingGui_SRCS} ${DrawingGuiView_SRCS} ${DrawingGuiViewProvider_SRCS})
target_link_libraries(DrawingGui ${DrawingGui_LIBS})


fc_target_copy_resource(DrawingGui 
    ${CMAKE_SOURCE_DIR}/src/Mod/Drawing
    ${CMAKE_BINARY_DIR}/Mod/Drawing
    InitGui.py)

SET(DrawingGuiIcon_SVG
    Resources/icons/actions/drawing-landscape.svg
)

fc_copy_sources(DrawingGui "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/Mod/Drawing" ${DrawingGuiIcon_SVG})

INSTALL(FILES ${DrawingGuiIcon_SVG} DESTINATION "${CMAKE_INSTALL_DATADIR}/Mod/Drawing/Resources/icons/actions")


SET_BIN_DIR(DrawingGui DrawingGui /Mod/Drawing)
SET_PYTHON_PREFIX_SUFFIX(DrawingGui)

INSTALL(TARGETS DrawingGui DESTINATION ${CMAKE_INSTALL_LIBDIR})

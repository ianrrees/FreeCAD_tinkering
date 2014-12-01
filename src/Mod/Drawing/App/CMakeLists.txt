if(MSVC)
    add_definitions(-DFCAppDrawing -DHAVE_ACOSH -DHAVE_ASINH -DHAVE_ATANH)
else(MSVC)
    add_definitions(-DHAVE_LIMITS_H -DHAVE_CONFIG_H -DHAVE_ACOSH -DHAVE_ATANH -DHAVE_ASINH)
endif(MSVC)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${Boost_INCLUDE_DIRS}
    ${QT_INCLUDE_DIR}
    ${OCC_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${XercesC_INCLUDE_DIRS}
    ${QT_QTCORE_INCLUDE_DIR}
)
link_directories(${OCC_LIBRARY_DIR})

set(Drawing_LIBS
    Measure
    Part
    FreeCADApp
)

message( DrawingApp generating XML FeatureTemplatePy )
generate_from_xml(FeatureTemplatePy)
message( DrawingApp generating XML FeatureParametricTemplatePy )
generate_from_xml(FeatureParametricTemplatePy)
message( DrawingApp generating XML Finished )

SET(Features_SRCS
    FeaturePage.cpp
    FeaturePage.h
    FeatureClip.cpp
    FeatureClip.h
    FeatureOrthoView.cpp
    FeatureOrthoView.h
    FeatureParametricTemplate.cpp
    FeatureParametricTemplate.h
    FeatureSVGTemplate.cpp
    FeatureSVGTemplate.h
    FeatureTemplate.cpp
    FeatureTemplate.h
    FeatureView.cpp
    FeatureView.h
    FeatureViewAnnotation.cpp
    FeatureViewAnnotation.h
    FeatureViewCollection.cpp
    FeatureViewCollection.h
    FeatureViewDimension.cpp
    FeatureViewDimension.h
    FeatureViewOrthographic.cpp
    FeatureViewOrthographic.h
    FeatureViewPart.cpp
    FeatureViewPart.h
    FeatureViewSection.cpp
    FeatureViewSection.h
    FeatureViewSymbol.cpp
    FeatureViewSymbol.h
    PageGroup.cpp
    PageGroup.h
)

SET(Drawing_SRCS
    AppDrawing.cpp
    AppDrawingPy.cpp
    PreCompiled.cpp
    PreCompiled.h
)

SET(DrawingGeometry_SRCS
    Geometry.cpp
    Geometry.h
    GeometryObject.cpp
    GeometryObject.h
)

SET(DrawingAlgos_SRCS
    DrawingExport.cpp
    DrawingExport.h
    FeatureProjection.cpp
    FeatureProjection.h
    ProjectionAlgos.cpp
    ProjectionAlgos.h
)

SET(Python_SRCS
    FeatureTemplatePy.xml
    FeatureTemplatePyImp.cpp
    FeatureParametricTemplatePy.xml
    FeatureParametricTemplatePyImp.cpp
)

SOURCE_GROUP("Mod" FILES ${Drawing_SRCS})
SOURCE_GROUP("Features" FILES ${Features_SRCS})
SOURCE_GROUP("Geometry" FILES ${DrawingGeometry_SRCS})
SOURCE_GROUP("Algorithms" FILES ${DrawingAlgos_SRCS})
SOURCE_GROUP("Python" FILES ${Python_SRCS})

SET(Drawing_Templates
    Templates/A0_Landscape_ISO7200.svg
    Templates/A0_Landscape_plain.svg
    Templates/A0_Portrait_plain.svg
    Templates/A1_Landscape_ISO7200.svg
    Templates/A1_Landscape_plain.svg
    Templates/A1_Portrait_plain.svg
    Templates/A2_Landscape_ISO7200.svg
    Templates/A2_Landscape_plain.svg
    Templates/A2_Portrait_plain.svg
    Templates/A3_Landscape.svg
    Templates/A3_Landscape_ISO7200.svg
    Templates/A3_Landscape_plain.svg
    Templates/A3_Portrait_plain.svg
    Templates/A4_Landscape.svg
    Templates/A4_Landscape_ISO7200.svg
    Templates/A4_Landscape_plain.svg
    Templates/A4_Portrait_ISO7200.svg
    Templates/A4_Portrait_plain.svg
)

if(MSVC)
    #add_definitions(-D_PreComp_)
    #GET_MSVC_PRECOMPILED_SOURCE("PreCompiled.cpp" Drawing_CPP_SRCS ${Drawing_SRCS} ${Features_SRCS} ${DrawingAlgos_SRCS})
    #ADD_MSVC_PRECOMPILED_HEADER(Drawing PreCompiled.h PreCompiled.cpp Drawing_CPP_SRCS)
endif(MSVC)

add_library(Drawing SHARED ${Drawing_SRCS} ${Features_SRCS} ${DrawingAlgos_SRCS}
                           ${DrawingGeometry_SRCS} ${Python_SRCS})
target_link_libraries(Drawing ${Drawing_LIBS})

SET(Drawing_Scripts
    Init.py
    DrawingAlgos.py
)

fc_target_copy_resource(Drawing 
    ${CMAKE_SOURCE_DIR}/src/Mod/Drawing
    ${CMAKE_BINARY_DIR}/Mod/Drawing
    ${Drawing_Scripts})

fc_target_copy_resource(Drawing 
    ${CMAKE_SOURCE_DIR}/src/Mod/Drawing
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/Mod/Drawing
    ${Drawing_Templates})

SET_BIN_DIR(Drawing Drawing /Mod/Drawing)
SET_PYTHON_PREFIX_SUFFIX(Drawing)

INSTALL(TARGETS Drawing DESTINATION ${CMAKE_INSTALL_LIBDIR})

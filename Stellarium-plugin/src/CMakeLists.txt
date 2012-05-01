
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src
  ${STELLARIUM_BUILD_DIR}
  ${STELLARIUM_SOURCE_DIR}
  ${STELLARIUM_SOURCE_DIR}/core
  ${STELLARIUM_SOURCE_DIR}/core/external
  ${STELLARIUM_SOURCE_DIR}/core/modules
  ${STELLARIUM_SOURCE_DIR}/gui
  ${STELLARIUM_SOURCE_DIR}/scripting
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_SOURCE_DIR}/src
  )

LINK_DIRECTORIES(${STELLARIUM_BUILD_DIR}/src)

SET(JeeScope_SRCS
  JeeScope.hpp
  JeeScope.cpp
  )

# Add here all the .ui files
SET(JeeScope_UIS
  )
QT4_WRAP_UI(JeeScope_UIS_H ${JeeScope_UIS})

# Compile the resource files
SET(JeeScope_RES
#  ${CMAKE_SOURCE_DIR}/examplePainter.qrc
  )
QT4_ADD_RESOURCES(JeeScope_RES_CXX ${JeeScope_RES})

# Add here all the files which require moc compilation
# that is, all the headers with SIGNAL/SLOTS/PLUGIN_INTERFACE etc..
SET(JeeScope_MOC_HDRS
  JeeScope.hpp
  )

# After this call, JeeScope_MOC_SRCS = moc_JeeScope.cxx
QT4_WRAP_CPP(JeeScope_MOC_SRCS ${JeeScope_MOC_HDRS})

SET(extLinkerOption ${FTDI_LIBRARIES} ${QT_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES} ${ICONV_LIBRARIES} ${INTL_LIBRARIES})

############### For building the dynamic library ######################
IF(BUILD_DYNAMIC_LIB)
 ADD_LIBRARY(JeeScope MODULE ${JeeScope_SRCS} ${JeeScope_MOC_SRCS} ${JeeScope_RES_CXX} ${JeeScope_UIS_H})

 IF(APPLE)
  FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
  MARK_AS_ADVANCED(OPENGL_LIBRARY)
  SET_TARGET_PROPERTIES(JeeScope PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" SUFFIX ".dylib")
 ENDIF(APPLE)

 IF(WIN32)
  SET_TARGET_PROPERTIES(JeeScope PROPERTIES LINK_FLAGS "-enable-runtime-pseudo-reloc -Wl,--allow-multiple-definition" )
  SET(StelMain stelMain)
 ELSE(WIN32)
  SET(StelMain )
 ENDIF(WIN32)

 TARGET_LINK_LIBRARIES(JeeScope ${StelMain} ${extLinkerOption})

 INSTALL(TARGETS JeeScope DESTINATION "modules/${PACKAGE}")
ENDIF(BUILD_DYNAMIC_LIB)

############### For building the static library ######################
IF(BUILD_STATIC_LIB)
 ADD_LIBRARY(JeeScope-static STATIC ${JeeScope_SRCS} ${JeeScope_MOC_SRCS} ${JeeScope_RES_CXX} ${JeeScope_UIS_H})
 SET_TARGET_PROPERTIES(JeeScope-static PROPERTIES OUTPUT_NAME "JeeScope")
 TARGET_LINK_LIBRARIES(JeeScope-static ${StelMain} ${extLinkerOption})
 SET_TARGET_PROPERTIES(JeeScope-static PROPERTIES COMPILE_FLAGS "-fPIC -DQT_STATICPLUGIN")
ENDIF(BUILD_STATIC_LIB)

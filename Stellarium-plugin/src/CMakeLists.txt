
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

SET(JeePointer_SRCS
  JeePointer.hpp
  JeePointer.cpp
  )

# Add here all the .ui files
SET(JeePointer_UIS
  )
QT4_WRAP_UI(JeePointer_UIS_H ${JeePointer_UIS})

# Compile the resource files
SET(JeePointer_RES
#  ${CMAKE_SOURCE_DIR}/examplePainter.qrc
  )
QT4_ADD_RESOURCES(JeePointer_RES_CXX ${JeePointer_RES})

# Add here all the files which require moc compilation
# that is, all the headers with SIGNAL/SLOTS/PLUGIN_INTERFACE etc..
SET(JeePointer_MOC_HDRS
  JeePointer.hpp
  )

# After this call, JeePointer_MOC_SRCS = moc_JeePointer.cxx
QT4_WRAP_CPP(JeePointer_MOC_SRCS ${JeePointer_MOC_HDRS})

SET(extLinkerOption ${FTDI_LIBRARY} ${QT_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES} ${ICONV_LIBRARIES} ${INTL_LIBRARIES})

############### For building the dynamic library ######################
IF(BUILD_DYNAMIC_LIB)
 ADD_LIBRARY(JeePointer MODULE ${JeePointer_SRCS} ${JeePointer_MOC_SRCS} ${JeePointer_RES_CXX} ${JeePointer_UIS_H})

 IF(APPLE)
  FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
  MARK_AS_ADVANCED(OPENGL_LIBRARY)
  SET_TARGET_PROPERTIES(JeePointer PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" SUFFIX ".dylib")
 ENDIF(APPLE)

 IF(WIN32)
  SET_TARGET_PROPERTIES(JeePointer PROPERTIES LINK_FLAGS "-enable-runtime-pseudo-reloc -Wl,--allow-multiple-definition" )
  SET(StelMain stelMain)
 ELSE(WIN32)
  SET(StelMain )
 ENDIF(WIN32)

 TARGET_LINK_LIBRARIES(JeePointer ${StelMain} ${extLinkerOption})

 INSTALL(TARGETS JeePointer DESTINATION "modules/${PACKAGE}")
ENDIF(BUILD_DYNAMIC_LIB)

############### For building the static library ######################
IF(BUILD_STATIC_LIB)
 ADD_LIBRARY(JeePointer-static STATIC ${JeePointer_SRCS} ${JeePointer_MOC_SRCS} ${JeePointer_RES_CXX} ${JeePointer_UIS_H})
 SET_TARGET_PROPERTIES(JeePointer-static PROPERTIES OUTPUT_NAME "JeePointer")
 TARGET_LINK_LIBRARIES(JeePointer-static ${StelMain} ${extLinkerOption})
 SET_TARGET_PROPERTIES(JeePointer-static PROPERTIES COMPILE_FLAGS "-fPIC -DQT_STATICPLUGIN")
ENDIF(BUILD_STATIC_LIB)

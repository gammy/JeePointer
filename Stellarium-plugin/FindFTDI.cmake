# Try to find ftdi
# Once done this will define
#
#  FTDI_FOUND - system has ftdi
#  FTDI_INCLUDE_DIR - ~ the ftdi include directory 
#  FTDI_LIBRARY - Link these to use ftdi
#
#  2010, Ze Ji
#
IF(WIN32)
FIND_PATH(FTDI_INCLUDE_DIR 
NAMES   ftdi.h
PATHS   $ENV{FTDI_DIR}/src
		"C:/roboskin/software_yarp_related/ftdi_usb/libftdi-0.18/src"
)

FIND_LIBRARY(FTDI_LIBRARY
NAMES ftdi
PATHS $ENV{FTDI_DIR}/src/Debug
	  "C:/roboskin/software_yarp_related/ftdi_usb/libftdi-0.18/src/Debug"
	  $ENV{FTDI_DIR}/src/Release
	  "C:/roboskin/software_yarp_related/ftdi_usb/libftdi-0.18/src/Release"
)
					   
ELSE(WIN32)  
FIND_PATH(FTDI_INCLUDE_DIR 
NAMES   ftdi.h
PATHS   /usr/local/include
        /usr/include
       /usr/include/libftdi
        /usr/local/include/libftdi
)

FIND_LIBRARY(FTDI_LIBRARY
NAMES ftdi
PATHS /usr/lib
      /usr/local/lib
)
ENDIF(WIN32)


IF (FTDI_LIBRARY)
    set(FTDI_FOUND TRUE)
ELSE (FTDI_LIBRARY)
    set(FTDI_FOUND FALSE)    
ENDIF (FTDI_LIBRARY)

set(FTDI_INCLUDE_DIR
    ${FTDI_INCLUDE_DIR}
)

#MESSAGE("----------ftdi: ${FTDI_INCLUDE_DIR} ftdilib: ${FTDI_LIBRARY}")

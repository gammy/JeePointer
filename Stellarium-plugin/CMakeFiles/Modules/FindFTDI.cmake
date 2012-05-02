MESSAGE(STATUS "Looking for ftdi")

IF(WIN32)
	MESSAGE(STATUS "Eep, no configuration for windows")
ELSE(WIN32)  
	FIND_PATH(FTDI_INCLUDE_DIR 
		NAMES ftdi.hpp
		PATHS /usr/local/include
		      /usr/include
		      /usr/include/libftdi
		      /usr/local/include/libftdi
	)

	FIND_LIBRARY(FTDI_LIBRARY 
		NAMES ftdipp
		PATHS /usr/lib64
		      /usr/local/lib64
		      /usr/lib
		      /usr/local/lib
	)
ENDIF(WIN32)


IF (FTDI_LIBRARY)
    set(FTDI_FOUND TRUE)
    MESSAGE(STATUS "Looking for ftdi - found")
ELSE (FTDI_LIBRARY)
    set(FTDI_FOUND FALSE)    
ENDIF (FTDI_LIBRARY)

set(FTDI_INCLUDE_DIR
    ${FTDI_INCLUDE_DIR}
)

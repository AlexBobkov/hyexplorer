# This module defines

# QT_PROPERTYBROWSER_LIBRARY
# QT_PROPERTYBROWSER_FOUND, if false, do not try to link to qt property browser
# QT_PROPERTYBROWSER_INCLUDE_DIR, where to find the headers

# to use this module, set variables to point to the qt property browser directory
# QT_PROPERTYBROWSER_DIR

# Header files are presumed to be included like
# #include <qtpropertybrowser.h>

SET(QT_PROPERTYBROWSER_DIR "" CACHE PATH "Qt property browser directory")

FIND_PATH(QT_PROPERTYBROWSER_INCLUDE_DIR qtpropertybrowser.h
	HINTS
		$ENV{QT_PROPERTYBROWSER_DIR}
		$ENV{QT_PROPERTYBROWSERIR}
	PATH_SUFFIXES
		include
		src
		QtPropertyBrowser
	PATHS
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
		/usr/freeware
		/usr/
		/usr/local
		${QT_PROPERTYBROWSER_DIR}
)

FIND_LIBRARY(QT_PROPERTYBROWSER_LIBRARY QtSolutions_PropertyBrowser-head Qt5Solutions_PropertyBrowser-head
	HINTS
		$ENV{QT_PROPERTYBROWSER_DIR}
		$ENV{QT_PROPERTYBROWSERIR}
	PATH_SUFFIXES
		lib
		lib/x86
		lib/x64
	PATHS
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
		/usr/freeware
		/usr/
		/usr/local
		${QT_PROPERTYBROWSER_DIR}
)

FIND_LIBRARY(QT_PROPERTYBROWSER_LIBRARY_DEBUG QtSolutions_PropertyBrowser-headd Qt5Solutions_PropertyBrowser-headd
	HINTS
		$ENV{QT_PROPERTYBROWSER_DIR}
		$ENV{QT_PROPERTYBROWSERIR}
	PATH_SUFFIXES
		lib
		lib/x86
		lib/x64
	PATHS
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
		/usr/freeware
		/usr/
		/usr/local
		${QT_PROPERTYBROWSER_DIR}
)

SET(QT_PROPERTYBROWSER_FOUND "NO")
IF(QT_PROPERTYBROWSER_LIBRARY AND QT_PROPERTYBROWSER_INCLUDE_DIR)
    SET(QT_PROPERTYBROWSER_FOUND "YES")
	IF(QT_PROPERTYBROWSER_LIBRARY_DEBUG)
		SET(QT_PROPERTYBROWSER_LIBRARIES optimized ${QT_PROPERTYBROWSER_LIBRARY} debug ${QT_PROPERTYBROWSER_LIBRARY_DEBUG})
	ELSE()
		SET(QT_PROPERTYBROWSER_LIBRARIES ${QT_PROPERTYBROWSER_LIBRARY})
	ENDIF()	
ENDIF(QT_PROPERTYBROWSER_LIBRARY AND QT_PROPERTYBROWSER_INCLUDE_DIR)
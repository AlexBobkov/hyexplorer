# This module defines

# OSG_LIBRARY
# OSG_FOUND, if false, do not try to link to osg
# OSG_INCLUDE_DIRS, where to find the headers
# OSG_INCLUDE_DIR, where to find the source headers
# OSG_GEN_INCLUDE_DIR, where to find the generated headers

# to use this module, set variables to point to the osg build
# directory, and source directory, respectively
# OSGDIR or OSG_SOURCE_DIR: osg source directory, typically OpenSceneGraph
# OSG_DIR or OSG_BUILD_DIR: osg build directory, place in which you've
#    built osg via cmake

# Header files are presumed to be included like
# #include <osg/PositionAttitudeTransform>
# #include <osgUtil/SceneView>

###### headers ######

MACRO( FIND_OSG_INCLUDE THIS_OSG_INCLUDE_DIR THIS_OSG_INCLUDE_FILE )

FIND_PATH( ${THIS_OSG_INCLUDE_DIR} ${THIS_OSG_INCLUDE_FILE}
    PATHS
        ${OSG_DIR}
        $ENV{OSG_SOURCE_DIR}
        $ENV{OSGDIR}
        $ENV{OSG_DIR}
        /usr/local/
        /usr/
        /sw/ # Fink
        /opt/local/ # DarwinPorts
        /opt/csw/ # Blastwave
        /opt/
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/
        ~/Library/Frameworks
        /Library/Frameworks
    PATH_SUFFIXES
        /include/
        /inc/
)

ENDMACRO( FIND_OSG_INCLUDE THIS_OSG_INCLUDE_DIR THIS_OSG_INCLUDE_FILE )

FIND_OSG_INCLUDE( OSG_GEN_INCLUDE_DIR   osg/Config )
FIND_OSG_INCLUDE( OSG_INCLUDE_DIR       osg/Node )

###### libraries ######

MACRO( FIND_OSG_LIBRARY MYLIBRARY MYLIBRARYNAME )

FIND_LIBRARY(${MYLIBRARY}
    NAMES
        ${MYLIBRARYNAME}
    PATHS
        ${OSG_DIR}
        $ENV{OSG_BUILD_DIR}
        $ENV{OSG_DIR}
        $ENV{OSGDIR}
        $ENV{OSG_ROOT}
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw
        /opt/local
        /opt/csw
        /opt
        [HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session\ Manager\\Environment;OSG_ROOT]/lib
        /usr/freeware
    PATH_SUFFIXES
        /lib/
        /lib64/
        /build/lib/
        /build/lib64/
        /Build/lib/
        /Build/lib64/
        /win/32/debug/lib/
        /win/32/release/lib/
     )

ENDMACRO(FIND_OSG_LIBRARY LIBRARY LIBRARYNAME)

FIND_OSG_LIBRARY( OSG_LIBRARY osg )
FIND_OSG_LIBRARY( OSG_LIBRARY_DEBUG osgd)

FIND_OSG_LIBRARY( OSGUTIL_LIBRARY osgUtil )
FIND_OSG_LIBRARY( OSGUTIL_LIBRARY_DEBUG osgUtild)

FIND_OSG_LIBRARY( OSGDB_LIBRARY osgDB )
FIND_OSG_LIBRARY( OSGDB_LIBRARY_DEBUG osgDBd)

FIND_OSG_LIBRARY( OSGTEXT_LIBRARY osgText )
FIND_OSG_LIBRARY( OSGTEXT_LIBRARY_DEBUG osgTextd )

FIND_OSG_LIBRARY( OSGTERRAIN_LIBRARY osgTerrain )
FIND_OSG_LIBRARY( OSGTERRAIN_LIBRARY_DEBUG osgTerraind )

FIND_OSG_LIBRARY( OSGFX_LIBRARY osgFX )
FIND_OSG_LIBRARY( OSGFX_LIBRARY_DEBUG osgFXd )

FIND_OSG_LIBRARY( OSGPARTICLE_LIBRARY osgParticle )
FIND_OSG_LIBRARY( OSGPARTICLE_LIBRARY_DEBUG osgParticled )

FIND_OSG_LIBRARY( OSGSIM_LIBRARY osgSim )
FIND_OSG_LIBRARY( OSGSIM_LIBRARY_DEBUG osgSimd )

FIND_OSG_LIBRARY( OSGVIEWER_LIBRARY osgViewer )
FIND_OSG_LIBRARY( OSGVIEWER_LIBRARY_DEBUG osgViewerd )

FIND_OSG_LIBRARY( OSGGA_LIBRARY osgGA )
FIND_OSG_LIBRARY( OSGGA_LIBRARY_DEBUG osgGAd )

FIND_OSG_LIBRARY( OSGWIDGET_LIBRARY osgWidget )
FIND_OSG_LIBRARY( OSGWIDGET_LIBRARY_DEBUG osgWidgetd )

FIND_OSG_LIBRARY( OSGSHADOW_LIBRARY osgShadow )
FIND_OSG_LIBRARY( OSGSHADOW_LIBRARY_DEBUG osgShadowd )

FIND_OSG_LIBRARY( OSGMANIPULATOR_LIBRARY osgManipulator )
FIND_OSG_LIBRARY( OSGMANIPULATOR_LIBRARY_DEBUG osgManipulatord )

FIND_OSG_LIBRARY( OSGQT_LIBRARY osgQt )
FIND_OSG_LIBRARY( OSGQT_LIBRARY_DEBUG osgQtd )

FIND_OSG_LIBRARY( OSGVOLUME_LIBRARY osgVolume )
FIND_OSG_LIBRARY( OSGVOLUME_LIBRARY_DEBUG osgVolumed )

FIND_OSG_LIBRARY( OSGANIMATION_LIBRARY osgAnimation )
FIND_OSG_LIBRARY( OSGANIMATION_LIBRARY_DEBUG osgAnimationd)

FIND_OSG_LIBRARY( OPENTHREADS_LIBRARY OpenThreads )
FIND_OSG_LIBRARY( OPENTHREADS_LIBRARY_DEBUG OpenThreadsd )

SET( OSG_FOUND "NO" )
IF( OSG_LIBRARY AND OSG_INCLUDE_DIR )
    SET( OSG_FOUND "YES" )
    SET( OSG_INCLUDE_DIRS ${OSG_INCLUDE_DIR} ${OSG_GEN_INCLUDE_DIR} )
    GET_FILENAME_COMPONENT( OSG_LIBRARIES_DIR ${OSG_LIBRARY} PATH )

    IF(OSG_LIBRARY_DEBUG)
        SET(OSG_LIBRARIES
            optimized ${OSG_LIBRARY} debug ${OSG_LIBRARY_DEBUG}
            optimized ${OSGUTIL_LIBRARY} debug ${OSGUTIL_LIBRARY_DEBUG}
            optimized ${OSGDB_LIBRARY} debug ${OSGDB_LIBRARY_DEBUG}
            optimized ${OSGTEXT_LIBRARY} debug ${OSGTEXT_LIBRARY_DEBUG}
            optimized ${OSGTERRAIN_LIBRARY} debug ${OSGTERRAIN_LIBRARY_DEBUG}
            optimized ${OSGFX_LIBRARY} debug ${OSGFX_LIBRARY_DEBUG}
            optimized ${OSGPARTICLE_LIBRARY} debug ${OSGPARTICLE_LIBRARY_DEBUG}
            optimized ${OSGSIM_LIBRARY} debug ${OSGSIM_LIBRARY_DEBUG}
            optimized ${OSGVIEWER_LIBRARY} debug ${OSGVIEWER_LIBRARY_DEBUG}
            optimized ${OSGGA_LIBRARY} debug ${OSGGA_LIBRARY_DEBUG}
            optimized ${OSGWIDGET_LIBRARY} debug ${OSGWIDGET_LIBRARY_DEBUG}
            optimized ${OSGSHADOW_LIBRARY} debug ${OSGSHADOW_LIBRARY_DEBUG}
            optimized ${OSGMANIPULATOR_LIBRARY} debug ${OSGMANIPULATOR_LIBRARY_DEBUG}
            optimized ${OSGQT_LIBRARY} debug ${OSGQT_LIBRARY_DEBUG}
            optimized ${OSGVOLUME_LIBRARY} debug ${OSGVOLUME_LIBRARY_DEBUG}
            optimized ${OSGANIMATION_LIBRARY} debug ${OSGANIMATION_LIBRARY_DEBUG}
            optimized ${OPENTHREADS_LIBRARY} debug ${OPENTHREADS_LIBRARY_DEBUG}
        )
    ELSE(OSG_LIBRARY_DEBUG)
        SET(OSG_LIBRARIES
            ${OSG_LIBRARY}
            ${OSGUTIL_LIBRARY}
            ${OSGDB_LIBRARY}
            ${OSGTEXT_LIBRARY}
            ${OSGTERRAIN_LIBRARY}
            ${OSGFX_LIBRARY}
            ${OSGPARTICLE_LIBRARY}
            ${OSGSIM_LIBRARY}
            ${OSGVIEWER_LIBRARY}
            ${OSGGA_LIBRARY}
            ${OSGWIDGET_LIBRARY}
            ${OSGSHADOW_LIBRARY}
            ${OSGMANIPULATOR_LIBRARY}
            ${OSGQT_LIBRARY}
            ${OSGVOLUME_LIBRARY}
            ${OSGANIMATION_LIBRARY}
            ${OPENTHREADS_LIBRARY}
        )
    ENDIF(OSG_LIBRARY_DEBUG)
ENDIF( OSG_LIBRARY AND OSG_INCLUDE_DIR )



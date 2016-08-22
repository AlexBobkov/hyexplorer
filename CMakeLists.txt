cmake_minimum_required(VERSION 3.0.0)

project(HyExplorer)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(CMAKE_DEBUG_POSTFIX "d")

include(cotire)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#######################################################################

find_package(OSG REQUIRED)
find_package(OsgEarth REQUIRED)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

find_package(QtPropertyBrowser REQUIRED)

find_package(Boost 1.55.0 REQUIRED)

#######################################################################

set(GEOPORTAL_MAJOR_VERSION 0 CACHE STRING "Major version")
set(GEOPORTAL_MINOR_VERSION 3 CACHE STRING "Minor version")
set(GEOPORTAL_PATCH_VERSION 1 CACHE STRING "Patch version")

configure_file(
    ${PROJECT_SOURCE_DIR}/src/geoPortal/Version.cpp.in
    ${PROJECT_SOURCE_DIR}/src/geoPortal/Version.cpp
)

#######################################################################

add_subdirectory(src/importAviris)
add_subdirectory(src/importAvirisOverviews)
add_subdirectory(src/importHyperion)
add_subdirectory(src/importHyperionOverviews)
add_subdirectory(src/geoPortal)

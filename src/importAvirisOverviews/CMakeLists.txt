set(HEADER_FILES
    Downloader.hpp
)

set(SRC_FILES
    Downloader.cpp
    Program.cpp
)

add_executable(ImportAvirisOverviews ${SRC_FILES} ${HEADER_FILES})

target_include_directories(ImportAvirisOverviews PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(ImportAvirisOverviews
    ${Boost_LIBRARIES}
    Qt5::Widgets
    Qt5::Sql
    Qt5::Network
)

install(TARGETS ImportAvirisOverviews RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

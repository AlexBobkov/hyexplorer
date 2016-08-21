set(HEADER_FILES
    Downloader.hpp
)

set(SRC_FILES
    Downloader.cpp
    Program.cpp
)

add_executable(ImportHyperionOverviews ${SRC_FILES} ${HEADER_FILES})

target_include_directories(ImportHyperionOverviews PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(ImportHyperionOverviews
    ${Boost_LIBRARIES}
    Qt5::Widgets
    Qt5::Sql
    Qt5::Network
)

install(TARGETS ImportHyperionOverviews RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

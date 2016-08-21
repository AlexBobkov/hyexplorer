set(HEADER_FILES
)

set(SRC_FILES
    Program.cpp
)

add_executable(ImportHyperion ${SRC_FILES} ${HEADER_FILES})

target_include_directories(ImportHyperion PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(ImportHyperion
    ${Boost_LIBRARIES}
    Qt5::Widgets
    Qt5::Sql
)

install(TARGETS ImportHyperion RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

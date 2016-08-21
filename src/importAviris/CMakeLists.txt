set(HEADER_FILES
    ${PROJECT_SOURCE_DIR}/include/AvirisCsvRow.hpp
)

set(SRC_FILES
    Program.cpp
)

add_executable(ImportAviris ${SRC_FILES} ${HEADER_FILES})

target_include_directories(ImportAviris PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(ImportAviris
    ${Boost_LIBRARIES}
    Qt5::Widgets
    Qt5::Sql
)

install(TARGETS ImportAviris RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

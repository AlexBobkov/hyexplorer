set(LIB_QT_UIS
    ui/MainWindow.ui
    ui/ProcessingWidget.ui
    ui/SceneOperationsWidget.ui
    ui/SettingsWidget.ui
)

qt5_wrap_ui(LIB_UI_HDRS ${LIB_QT_UIS})

set(HEADER_FILES
    ${LIB_UI_HDRS}
    ClipInfo.hpp
    DataManager.hpp
    DataSet.hpp
    EventHandlers.hpp
    MainWindow.hpp
    MetadataWidget.hpp
    Operations.hpp
    ProcessingWidget.hpp
    ProxyModel.hpp
    Scene.hpp
    SceneOperationsWidget.hpp
    SensorQuery.hpp
    SettingsWidget.hpp
    Storage.hpp
    TableModel.hpp
    Utils.hpp
    Version.hpp
)

set(SRC_FILES
    ClipInfo.cpp
    DataManager.cpp
    DataSet.cpp
    EventHandlers.cpp
    MainWindow.cpp
    MetadataWidget.cpp
    Operations.cpp
    ProcessingWidget.cpp
    Program.cpp
    ProxyModel.cpp
    Scene.cpp
    SceneOperationsWidget.cpp
    SensorQuery.cpp
    SettingsWidget.cpp
    Storage.cpp
    TableModel.cpp
    Utils.cpp
)

add_executable(HyExplorer ${SRC_FILES} ${HEADER_FILES})

target_compile_definitions(HyExplorer PRIVATE BOOST_ALL_DYN_LINK)

target_include_directories(HyExplorer PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${OSG_INCLUDE_DIRS}
    ${OSGEARTH_INCLUDE_DIRS}
)

target_link_libraries(HyExplorer
    ${OSG_LIBRARIES}
    ${OSGEARTH_LIBRARIES}
    ${Boost_LIBRARIES}
    Qt5::Widgets
    Qt5::OpenGL
    Qt5::Network
    Qt5::Sql
    Qt::QtPropertyBrowser
)

install(TARGETS HyExplorer RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

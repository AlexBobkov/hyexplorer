SET(APP_NAME GeoPortal)

set(LIB_QT_UIS	
	ui/MainWindow.ui
	ui/ProcessingWidget.ui
	ui/SceneOperationsWidget.ui
	ui/SettingsWidget.ui
)

QT5_WRAP_UI(LIB_UI_HDRS ${LIB_QT_UIS})

SET(HEADER_FILES
	${LIB_UI_HDRS}
	ClipInfo.hpp
	DataManager.hpp
	DataSet.hpp
	DownloadManager.hpp	
	MainWindow.hpp
	MetadataWidget.hpp
	ProcessingWidget.hpp
	ProxyModel.hpp
	Scene.hpp
	SceneOperationsWidget.hpp
	SensorQuery.hpp
	SettingsWidget.hpp
	Storage.hpp
	TableModel.hpp
)

SET(SRC_FILES
	ClipInfo.cpp
	DataManager.cpp
	DataSet.cpp
	DownloadManager.cpp	
	MainWindow.cpp
	MetadataWidget.cpp
	ProcessingWidget.cpp
	Program.cpp
	ProxyModel.cpp
	Scene.cpp
	SceneOperationsWidget.cpp
	SensorQuery.cpp
	SettingsWidget.cpp
	Storage.cpp
	TableModel.cpp
)

INCLUDE_DIRECTORIES(
	${PROJECT_SOURCE_DIR}/include
	${CMAKE_INCLUDE_PATH}	
	${GDAL_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${QT_INCLUDES}
	${OSG_INCLUDE_DIRS}	
	${OSGEARTH_INCLUDE_DIRS}
	${QT_PROPERTYBROWSER_INCLUDE_DIR}
)

ADD_EXECUTABLE(${APP_NAME} ${SRC_FILES} ${HEADER_FILES})

TARGET_LINK_LIBRARIES(${APP_NAME}	
	${OSGEARTH_LIBRARIES}
	${QT_PROPERTYBROWSER_LIBRARIES}
)

LINK_WITH_VARIABLES(${APP_NAME}
	OPENTHREADS_LIBRARY
	OSG_LIBRARY
	OSGUTIL_LIBRARY
	OSGDB_LIBRARY
	OSGVIEWER_LIBRARY
	OSGTEXT_LIBRARY
	OSGPARTICLE_LIBRARY
	OSGGA_LIBRARY
	OSGVOLUME_LIBRARY
	OSGQT_LIBRARY
	GDAL_LIBRARY
)

IF(Qt5Widgets_FOUND)
    QT5_USE_MODULES(${APP_NAME} Gui Widgets OpenGL Sql Xml Network)
ENDIF(Qt5Widgets_FOUND)

INSTALL(TARGETS ${APP_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

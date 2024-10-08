project(demodanalyzer)

set(demodanalyzer_SOURCES
	demodanalyzer.cpp
    demodanalyzersettings.cpp
    demodanalyzerplugin.cpp
    demodanalyzerworker.cpp
    demodanalyzerwebapiadapter.cpp
)

set(demodanalyzer_HEADERS
	demodanalyzer.h
    demodanalyzersettings.h
    demodanalyzerplugin.h
    demodanalyzerworker.h
    demodanalyzerwebapiadapter.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/swagger/sdrangel/code/qt5/client
)

if(NOT SERVER_MODE)
    set(demodanalyzer_SOURCES
        ${demodanalyzer_SOURCES}
        demodanalyzergui.cpp
        demodanalyzergui.ui
    )
    set(demodanalyzer_HEADERS
        ${demodanalyzer_HEADERS}
	    demodanalyzergui.h
    )

    set(TARGET_NAME ${PLUGINS_PREFIX}featuredemodanalyzer)
    set(TARGET_LIB "Qt::Widgets")
    set(TARGET_LIB_GUI "sdrgui")
    set(INSTALL_FOLDER ${INSTALL_PLUGINS_DIR})
else()
    set(TARGET_NAME ${PLUGINSSRV_PREFIX}featuredemodanalyzersrv)
    set(TARGET_LIB "")
    set(TARGET_LIB_GUI "")
    set(INSTALL_FOLDER ${INSTALL_PLUGINSSRV_DIR})
endif()

if(NOT Qt6_FOUND)
    add_library(${TARGET_NAME} ${demodanalyzer_SOURCES})
else()
    qt_add_plugin(${TARGET_NAME} CLASS_NAME DemodAnalyzerPlugin ${demodanalyzer_SOURCES})
endif()

if(NOT BUILD_SHARED_LIBS)
    set_property(GLOBAL APPEND PROPERTY STATIC_PLUGINS_PROPERTY ${TARGET_NAME})
endif()

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt::Core
    ${TARGET_LIB}
	sdrbase
	${TARGET_LIB_GUI}
)

install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_FOLDER})

# Install debug symbols
if (WIN32)
    install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}> CONFIGURATIONS Debug RelWithDebInfo DESTINATION ${INSTALL_FOLDER} )
endif()

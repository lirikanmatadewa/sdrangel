project(widebandscanner)

set(widebandscanner_SOURCES
    widebandscanner.cpp
    widebandscannersettings.cpp
    widebandscannerbaseband.cpp
    widebandscannersink.cpp
    widebandscannerplugin.cpp
    widebandscannerwebapiadapter.cpp
)

set(widebandscanner_HEADERS
    widebandscanner.h
    widebandscannersettings.h
    widebandscannerbaseband.h
    widebandscannersink.h
    widebandscannerplugin.h
    widebandscannerwebapiadapter.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/swagger/sdrangel/code/qt5/client
)

if(NOT SERVER_MODE)
    set(widebandscanner_SOURCES
        ${widebandscanner_SOURCES}
        widebandscannergui.cpp
        widebandscannergui.ui
        widebandscanneraddrangedialog.cpp
        widebandscanneraddrangedialog.ui
    )
    set(widebandscanner_HEADERS
        ${widebandscanner_HEADERS}
        widebandscannergui.h
        widebandscanneraddrangedialog.h
    )

    set(TARGET_NAME widebandscanner)
    set(TARGET_LIB "Qt::Widgets")
    set(TARGET_LIB_GUI "sdrgui")
    set(INSTALL_FOLDER ${INSTALL_PLUGINS_DIR})
else()
    set(TARGET_NAME widebandscannersrv)
    set(TARGET_LIB "")
    set(TARGET_LIB_GUI "")
    set(INSTALL_FOLDER ${INSTALL_PLUGINSSRV_DIR})
endif()

add_library(${TARGET_NAME} SHARED
    ${widebandscanner_SOURCES}
)

target_link_libraries(${TARGET_NAME}
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

project(gsmchannelyzer)

set(gsmchannelyzer_SOURCES
    gsmchannelyzer.cpp
    gsmchannelyzersettings.cpp
    gsmchannelyzerbaseband.cpp
    gsmchannelyzersink.cpp
    gsmchannelyzerplugin.cpp
    gsmchannelyzerwebapiadapter.cpp
)

set(gsmchannelyzer_HEADERS
    gsmchannelyzer.h
    gsmchannelyzersettings.h
    gsmchannelyzerbaseband.h
    gsmchannelyzersink.h
    gsmchannelyzerplugin.h
    gsmchannelyzerwebapiadapter.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/swagger/sdrangel/code/qt5/client
)

if(NOT SERVER_MODE)
    set(gsmchannelyzer_SOURCES
        ${gsmchannelyzer_SOURCES}
        gsmchannelyzergui.cpp
        gsmchannelyzergui.ui
        gsmchannelyzeraddrangedialog.cpp
        gsmchannelyzeraddrangedialog.ui
    )
    set(gsmchannelyzer_HEADERS
        ${gsmchannelyzer_HEADERS}
        gsmchannelyzergui.h
        gsmchannelyzeraddrangedialog.h
    )

    set(TARGET_NAME gsmchannelyzer)
    set(TARGET_LIB "Qt::Widgets")
    set(TARGET_LIB_GUI "sdrgui")
    set(INSTALL_FOLDER ${INSTALL_PLUGINS_DIR})
else()
    set(TARGET_NAME gsmchannelyzersrv)
    set(TARGET_LIB "")
    set(TARGET_LIB_GUI "")
    set(INSTALL_FOLDER ${INSTALL_PLUGINSSRV_DIR})
endif()

add_library(${TARGET_NAME} SHARED
    ${gsmchannelyzer_SOURCES}
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

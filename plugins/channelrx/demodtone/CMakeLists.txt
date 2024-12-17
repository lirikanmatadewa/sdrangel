project(tone)

set(tone_SOURCES
	tonedemod.cpp
    tonedemodsettings.cpp
    tonedemodsink.cpp
    tonedemodbaseband.cpp
    tonedemodwebapiadapter.cpp
	toneplugin.cpp
)

set(tone_HEADERS
	tonedemod.h
    tonedemodsettings.h
    tonedemodsink.h
    tonedemodbaseband.h
    tonedemodwebapiadapter.h
	toneplugin.h
)

include_directories(
        ${CMAKE_SOURCE_DIR}/swagger/sdrangel/code/qt5/client
)

if(NOT SERVER_MODE)
    set(tone_SOURCES
        ${tone_SOURCES}
        tonedemodgui.cpp
	    tonedemodgui.ui
    )
    set(tone_HEADERS
        ${tone_HEADERS}
        tonedemodgui.h
    )

    set(TARGET_NAME demodtone)
    set(TARGET_LIB "Qt::Widgets")
    set(TARGET_LIB_GUI "sdrgui")
    set(INSTALL_FOLDER ${INSTALL_PLUGINS_DIR})
else()
    set(TARGET_NAME demodtonesrv)
    set(TARGET_LIB "")
    set(TARGET_LIB_GUI "")
    set(INSTALL_FOLDER ${INSTALL_PLUGINSSRV_DIR})
endif()

add_library(${TARGET_NAME} SHARED
	${tone_SOURCES}
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

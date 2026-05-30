project(cw)

set(cw_SOURCES
	cwdemod.cpp
    cwdemodsettings.cpp
    cwdemodsink.cpp
    cwdemodbaseband.cpp
    cwdemodwebapiadapter.cpp
	cwplugin.cpp
)

set(cw_HEADERS
	cwdemod.h
    cwdemodsettings.h
    cwdemodsink.h
    cwdemodbaseband.h
    cwdemodwebapiadapter.h
	cwplugin.h
)

include_directories(
        ${CMAKE_SOURCE_DIR}/swagger/sdrangel/code/qt5/client
)

if(NOT SERVER_MODE)
    set(cw_SOURCES
        ${cw_SOURCES}
        cwdemodgui.cpp
        cwdemodgui.ui
    )
    set(cw_HEADERS
        ${cw_HEADERS}
        cwdemodgui.h
    )
    set(TARGET_NAME ${PLUGINS_PREFIX}demodcw)
    set(TARGET_LIB "Qt::Widgets")
    set(TARGET_LIB_GUI "sdrgui")
    set(INSTALL_FOLDER ${INSTALL_PLUGINS_DIR})
else()
    set(TARGET_NAME ${PLUGINSSRV_PREFIX}demodcwsrv)
    set(TARGET_LIB "")
    set(TARGET_LIB_GUI "")
    set(INSTALL_FOLDER ${INSTALL_PLUGINSSRV_DIR})
endif()

if(NOT Qt6_FOUND)
    # Explicitly build the demodcw plugin as a shared library so it
    # always produces linker-created artifacts (PDBs on Windows).
    add_library(${TARGET_NAME} SHARED ${cw_SOURCES})
else()
    qt_add_plugin(${TARGET_NAME} CLASS_NAME CWPlugin ${cw_SOURCES})
endif()

if(NOT BUILD_SHARED_LIBS)
    set_property(GLOBAL APPEND PROPERTY STATIC_PLUGINS_PROPERTY ${TARGET_NAME})
endif()

target_link_libraries(${TARGET_NAME} PRIVATE
    Qt::Core
    ${TARGET_LIB}
	sdrbase
	${TARGET_LIB_GUI}
    swagger
)

install(TARGETS ${TARGET_NAME} DESTINATION ${INSTALL_FOLDER})

# Install debug symbols
if (WIN32)
    # Install PDB for Debug/RelWithDebInfo when the target produces linker
    # artifacts. Do not unconditionally install a "stripped" PDB because
    # some build configurations do not generate it and that causes CPack to
    # fail during packaging.
    get_target_property(_tgt_type ${TARGET_NAME} TYPE)
    if(_tgt_type STREQUAL "EXECUTABLE" OR _tgt_type STREQUAL "SHARED_LIBRARY" OR _tgt_type STREQUAL "MODULE_LIBRARY")
        install(FILES $<TARGET_PDB_FILE:${TARGET_NAME}>
                CONFIGURATIONS Debug RelWithDebInfo
                DESTINATION ${INSTALL_FOLDER} )
    endif()
endif()

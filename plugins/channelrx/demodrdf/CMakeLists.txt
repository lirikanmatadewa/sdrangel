project(rdf)

set(rdf_SOURCES
	rdfdemod.cpp
    rdfdemodsettings.cpp
    rdfdemodsink.cpp
    rdfdemodbaseband.cpp
    rdfdemodwebapiadapter.cpp
	rdfplugin.cpp
)

set(rdf_HEADERS
	rdfdemod.h
    rdfdemodsettings.h
    rdfdemodsink.h
    rdfdemodbaseband.h
    rdfdemodwebapiadapter.h
	rdfplugin.h
)

include_directories(
        ${CMAKE_SOURCE_DIR}/swagger/sdrangel/code/qt5/client
)

if(NOT SERVER_MODE)
    set(rdf_SOURCES
        ${rdf_SOURCES}
        rdfdemodgui.cpp
	    rdfdemodgui.ui
    )
    set(rdf_HEADERS
        ${rdf_HEADERS}
        rdfdemodgui.h
    )

    set(TARGET_NAME demodrdf)
    set(TARGET_LIB "Qt::Widgets")
    set(TARGET_LIB_GUI "sdrgui")
    set(INSTALL_FOLDER ${INSTALL_PLUGINS_DIR})
else()
    set(TARGET_NAME demodrdfsrv)
    set(TARGET_LIB "")
    set(TARGET_LIB_GUI "")
    set(INSTALL_FOLDER ${INSTALL_PLUGINSSRV_DIR})
endif()

add_library(${TARGET_NAME} SHARED
	${rdf_SOURCES}
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

# Licensing Framework Library (subdirectory - no cmake_minimum_required needed)
set(licensing_HEADERS
    license.hpp
    licensemanager.h
    aes_key.h
    public_key.h
)

set(licensing_SOURCES
    license.cpp
    licensemanager.cpp
    aes_key.cpp
    public_key.cpp
)

# Find required packages
# Use static OpenSSL to avoid shipping OpenSSL DLLs with the licensing library
if(WIN32)
    set(OPENSSL_WIN_ROOT "C:/Program Files (x86)/OpenSSL-WinUniversal")
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_WIN_ROOT}/include/x64" CACHE PATH "" FORCE)
    set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_WIN_ROOT}/lib/VC/x64/MD/libcrypto_static.lib" CACHE FILEPATH "" FORCE)
    set(OPENSSL_SSL_LIBRARY "${OPENSSL_WIN_ROOT}/lib/VC/x64/MD/libssl_static.lib" CACHE FILEPATH "" FORCE)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()
find_package(OpenSSL REQUIRED)
if(ENABLE_QT6)
    find_package(Qt6 REQUIRED COMPONENTS Core Network)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Network)
endif()

# Create the licensing framework library
add_library(licensing STATIC ${licensing_SOURCES} ${licensing_HEADERS})

# Set target properties
target_include_directories(licensing PUBLIC .)
target_link_libraries(licensing
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# On Windows, link additional system libraries needed for static OpenSSL
if(WIN32)
    target_link_libraries(licensing
        ws2_32      # Windows Sockets API
        crypt32     # Windows Cryptography API
        user32      # Windows User API
        advapi32    # Windows Advanced API
        bcrypt      # Windows Cryptography API (newer)
        ncrypt      # Windows Key Storage
    )
endif()

if(ENABLE_QT6)
    target_link_libraries(licensing
        Qt::Core
        Qt::Network
    )
else()
    target_link_libraries(licensing
        Qt5::Core
        Qt5::Network
    )
endif()

# Ensure the library is available to other targets
set_target_properties(licensing PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)
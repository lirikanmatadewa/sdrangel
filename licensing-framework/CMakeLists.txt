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
    # Prefer a local OpenSSL installation for Windows if it exists.
    set(OPENSSL_WIN_ROOT "C:/Program Files (x86)/OpenSSL-WinUniversal")

    # Only force the OpenSSL variables if the expected include and lib
    # files actually exist on the machine. For CI or developer machines
    # where OpenSSL is installed in a different location, fall back to
    # the normal find_package search so CMake can locate the system
    # OpenSSL installation.
    set(_openssl_inc "${OPENSSL_WIN_ROOT}/include/x64")
    set(_openssl_crypto_lib "${OPENSSL_WIN_ROOT}/lib/VC/x64/MD/libcrypto_static.lib")
    set(_openssl_ssl_lib "${OPENSSL_WIN_ROOT}/lib/VC/x64/MD/libssl_static.lib")

    if(EXISTS "${_openssl_inc}" AND EXISTS "${_openssl_crypto_lib}" AND EXISTS "${_openssl_ssl_lib}")
        set(OPENSSL_INCLUDE_DIR "${_openssl_inc}" CACHE PATH "" FORCE)
        set(OPENSSL_CRYPTO_LIBRARY "${_openssl_crypto_lib}" CACHE FILEPATH "" FORCE)
        set(OPENSSL_SSL_LIBRARY "${_openssl_ssl_lib}" CACHE FILEPATH "" FORCE)
        set(OPENSSL_USE_STATIC_LIBS TRUE)
    else()
        message(WARNING "OpenSSL not found under ${OPENSSL_WIN_ROOT}; leaving OpenSSL search to find_package(). If you intend to use the bundled OpenSSL-WinUniversal, install it or adjust OPENSSL_WIN_ROOT.")
    endif()
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
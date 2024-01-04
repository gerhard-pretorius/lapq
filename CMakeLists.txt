
#-----------------------------------------------------------------------------
project(lapq)

set(LIB lapq)
set(LIB_MAJOR 1)
set(LIB_MINOR 0)
set(LIB_PATCH 1)
set(LIB_VERSION ${LIB_MAJOR}.${LIB_MINOR}.${LIB_PATCH})

#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# CMake requirements
cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)
cmake_policy(SET CMP0017 OLD)

enable_testing()
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


#-----------------------------------------------------------------------------
# Compiler flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
add_definitions(-DASIO_STANDALONE -DASIO_HAS_STD_CHRONO)


#-----------------------------------------------------------------------------
# Library path
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE on)
include_directories(${CMAKE_INSTALL_PREFIX}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
link_directories(${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")

set(REQ_LIBRARIES stdc++fs crypto ssl pthread rt)


#-----------------------------------------------------------------------------
# Configure CCache if available
#find_program(CCACHE_FOUND ccache)
#if(CCACHE_FOUND)
#        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
#        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
#endif(CCACHE_FOUND)


#-----------------------------------------------------------------------------
# ASIO

if(NOT DEFINED ASIO_PREFIX)
    set(ASIO_PREFIX /usr/local)
endif(NOT DEFINED ASIO_PREFIX)

if(NOT DEFINED ASIO_INSTALL)
    set(ASIO_INSTALL ${ASIO_PREFIX}/include)
endif(NOT DEFINED ASIO_INSTALL)

include_directories(${ASIO_INSTALL})
message("ASIO_INSTALL=${ASIO_INSTALL}")

#-----------------------------------------------------------------------------
# TTMATH

if(NOT DEFINED TTMATH_PREFIX)
    set(TTMATH_PREFIX /usr/local)
endif(NOT DEFINED TTMATH_PREFIX)

if(NOT DEFINED TTMATH_INSTALL)
    set(TTMATH_INSTALL ${TTMATH_PREFIX}/include)
endif(NOT DEFINED TTMATH_INSTALL)

include_directories(${TTMATH_INSTALL})
message("TTMATH_INSTALL=${TTMATH_INSTALL}")




#-----------------------------------------------------------------------------
# Postgres
# TODO: use `pg_config --includedir` instead
include_directories(SYSTEM /usr/include/postgresql)


#-----------------------------------------------------------------------------
add_subdirectory(src)
add_subdirectory(test)


#-----------------------------------------------------------------------------
# Documentation

find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_subdirectory(doc)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation" VERBATIM
    )
    
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ COMPONENT doc DESTINATION doc)
endif(DOXYGEN_FOUND)

cmake_minimum_required(VERSION 3.1)
project(witmotion-uart-qt)

# VERSIONING
execute_process(COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE COMMIT_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(STRINGS "version" PROJECT_VERSION)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
set(PROJECT_VERSION "${PROJECT_VERSION}")
else(CMAKE_BUILD_TYPE STREQUAL "Release")
set(PROJECT_VERSION "${PROJECT_VERSION}~dev_${COMMIT_ID}")
endif(CMAKE_BUILD_TYPE STREQUAL "Release")
message(STATUS "Building version ${PROJECT_VERSION}")
configure_file(config/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/witmotion/version.h
    )

set (CMAKE_CXX_STANDARD 14)
include(CheckIncludeFile)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
if(NOT HAVE_INTTYPES_H)
    message(FATAL_ERROR "\'inttypes.h\' include file required!. Please check your toolchain!")
endif(NOT HAVE_INTTYPES_H)
include_directories(include
    ${CMAKE_CURRENT_BINARY_DIR}
    )
find_package(Qt5 REQUIRED COMPONENTS Core SerialPort)
set(CMAKE_AUTOMOC ON)

# OPTIONS
option(BUILD_EXAMPLES "Whether build or not the set of example applications" ON)
option(BUILD_DOCS "Whether build or not HTML documentation" OFF)

# LIBRARY
qt5_wrap_cpp(MOC_SOURCES
    include/witmotion/types.h
    include/witmotion/serial.h
    )
set(LIBRARY_SOURCES
    ${MOC_SOURCES}
    src/util.cpp
    src/serial.cpp
    )
add_library(witmotion-uart SHARED ${LIBRARY_SOURCES})
target_link_libraries(witmotion-uart Qt5::Core Qt5::SerialPort)

qt5_wrap_cpp(MOC_ENUMERATOR
    include/witmotion/message-enumerator.h
    )
add_executable(message-enumerator
    src/message-enumerator.cpp
    ${MOC_ENUMERATOR}
    )
target_link_libraries(message-enumerator
    Qt5::Core Qt5::SerialPort
    witmotion-uart
    )

# WT31N
qt5_wrap_cpp(MOC_WT31N
    include/witmotion/wt31n-uart.h
    )
add_library(witmotion-wt31n SHARED
    ${MOC_WT31N}
    src/wt31n-uart.cpp
    )
target_link_libraries(witmotion-wt31n witmotion-uart)
add_executable(witmotionctl-wt31n
    ${MOC_WT31N}
    src/wt31n-control.cpp
    )
target_link_libraries(witmotionctl-wt31n witmotion-wt31n)

# EXAMPLES
if(BUILD_EXAMPLES)
    add_executable(wt31n-calibration
        examples/wt31n/calibration-standalone.cpp
        )
    target_link_libraries(wt31n-calibration Qt5::Core Qt5::SerialPort)

    add_executable(wt31n-standalone-decoder
        examples/wt31n/witmotion-wt31n-uart-test.cpp
        )
    target_link_libraries(wt31n-standalone-decoder Qt5::Core Qt5::SerialPort)
endif(BUILD_EXAMPLES)

# DOCUMENTATION
if(BUILD_DOCS)
    include(cmake/doxygen-generator.cmake)
    if(${PROJECT_NAME}_DOCS_EXISTS)
        add_custom_target(docs ALL DEPENDS doc-${PROJECT_NAME})
    endif(${PROJECT_NAME}_DOCS_EXISTS)
endif(BUILD_DOCS)

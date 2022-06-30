cmake_minimum_required(VERSION 3.1)
project(witmotion-uart-qt)
set (CMAKE_CXX_STANDARD 14)
include(CheckIncludeFile)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
if(NOT HAVE_INTTYPES_H)
    message(FATAL_ERROR "\'inttypes.h\' include file required!. Please check your toolchain!")
endif(NOT HAVE_INTTYPES_H)
include_directories(include)
find_package(Qt5 REQUIRED COMPONENTS Core SerialPort)
set(CMAKE_AUTOMOC ON)
qt5_wrap_cpp(MOC_SOURCES
    include/witmotion/types.h
    include/witmotion/serial.h
    include/witmotion/wt31n-uart.h
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
option(BUILD_EXAMPLES "Whether build or not the set of example applications" ON)
if(BUILD_EXAMPLES)
    add_executable(wt31n-calibration
        examples/wt31n/calibration-standalone.cpp
        )
    target_link_libraries(wt31n-calibration Qt5::Core Qt5::SerialPort)

    add_executable(wt31n-standalone-decoder
        examples/wt31n/witmotion-wt31n-uart-test.cpp
        )
    target_link_libraries(wt31n-standalone-decoder Qt5::Core Qt5::SerialPort)

#    add_executable(wt31n-monitor
#        examples/wt31n/monitor.cpp
#        )
#    target_link_libraries(wt31n-monitor witmotion-uart)
endif(BUILD_EXAMPLES)

include(cmake/doxygen-generator.cmake)
if(${PROJECT_NAME}_DOCS_EXISTS)
    add_custom_target(docs ALL DEPENDS doc-${PROJECT_NAME})
endif(${PROJECT_NAME}_DOCS_EXISTS)

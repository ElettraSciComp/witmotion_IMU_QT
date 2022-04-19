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
    src/wt31n-uart.cpp
    )
add_library(witmotion-wt31n-uart SHARED ${LIBRARY_SOURCES})
target_link_libraries(witmotion-wt31n-uart Qt5::Core Qt5::SerialPort)

add_executable(witmotion-wt31n-calibration
    src/calibration-standalone.cpp
    )
target_link_libraries(witmotion-wt31n-calibration Qt5::Core Qt5::SerialPort)

add_executable(witmotion-wt31n-uart-test
    tests/witmotion-wt31n-uart-test.cpp
    )
target_link_libraries(witmotion-wt31n-uart-test Qt5::Core Qt5::SerialPort)

enable_testing()
add_executable(library-init-test
    tests/library-init-test.cpp
    )
target_link_libraries(library-init-test
    Qt5::Core Qt5::SerialPort
    witmotion-wt31n-uart
    )
add_test(NAME library-init-test
    COMMAND library-init-test
    )

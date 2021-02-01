cmake_minimum_required(VERSION 3.16)
project(CourseC C)

set(CMAKE_C_STANDARD 11)

add_executable(CourseC main.c)

find_path(LIBUSB_INCLUDE_DIR
        NAMES libusb.h
        PATH_SUFFIXES "include" "libusb" "libusb-1.0")

find_library(LIBUSB_LIBRARY
        NAMES usb
        PATH_SUFFIXES "lib" "lib32" "lib64" "usb-1.0.so")

include_directories( ${LIBUSB_INCLUDE_DIR})
link_libraries( ${LIBUSB_LIBRARY})


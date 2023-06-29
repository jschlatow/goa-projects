cmake_minimum_required(VERSION 3.10...3.21)

set(LIB_VERSION "1.0.0")

project(lvgl_support VERSION ${LIB_VERSION} LANGUAGES CXX)

set(LIB_SRCS lvgl_support.cc)

add_library(lvgl_support SHARED lvgl_support.cc)

target_include_directories(lvgl_support PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>")

set_target_properties(
  lvgl_support
  PROPERTIES OUTPUT_NAME lvgl_support
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
             PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/lvgl_support.h")

install(
  TARGETS lvgl_support
  LIBRARY DESTINATION "lib"
  PUBLIC_HEADER DESTINATION "include")

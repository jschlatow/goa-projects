cmake_minimum_required(VERSION 3.10)
project(vfs_tap)

set(LIB_SRCS vfs_tap.cc)

add_library(vfs_tap SHARED ${LIB_SRCS})

# remove 'lib' prefix
set_target_properties(vfs_tap PROPERTIES PREFIX "")

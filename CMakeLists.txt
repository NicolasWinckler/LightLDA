cmake_minimum_required(VERSION 2.8)

PROJECT(LIGHTLDA)

OPTION(USE_HDFS "won't use hdfs on default, set ON to enable" OFF)

set(USE_ZMQ_PATH "~/quanox/soft/")
set(USE_MULTIVERSO_PATH "~/quanox/soft/DMTK/Multiverso_inst")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
find_package(MPI REQUIRED)
find_package(ZeroMQ 4.1.3 REQUIRED)
find_package(Multiverso REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -fno-omit-frame-pointer")

if(USE_HDFS)
    ADD_DEFINITIONS(-DMULTIVERSO_USE_HDFS)
    set(JVM_LIB /usr/local/java/default/jre/lib/amd64/server)
    set(HDFS_LIB /usr/local/hadoop/lib/native)
    LINK_DIRECTORIES(${HDFS_LIB})
    LINK_DIRECTORIES(${JVM_LIB})
endif(USE_HDFS)

#include_directories(${PROJECT_SOURCE_DIR}/include)

#set(MULTIVERSO_DIR ${PROJECT_SOURCE_DIR})
ADD_SUBDIRECTORY(src)
#ADD_SUBDIRECTORY(inference)
#ADD_SUBDIRECTORY(preprocess)



# uninstall target
#configure_file(
#    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#    IMMEDIATE @ONLY)

#add_custom_target(uninstall
#    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

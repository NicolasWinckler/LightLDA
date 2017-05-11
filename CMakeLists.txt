cmake_minimum_required(VERSION 2.8)

PROJECT(LIGHTLDA)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

#set(USE_ZMQ_PATH "${CMAKE_SOURCE_DIR}/multiverso/third_party")
#set(MPI_CXX_COMPILER "${CMAKE_SOURCE_DIR}/multiverso/third_party/bin/mpicxx")
#set(USE_MULTIVERSO_PATH "${CMAKE_SOURCE_DIR}/multiverso")

find_package(MPI REQUIRED)
find_package(ZeroMQ 4.1.3 REQUIRED)
#find_package(Multiverso REQUIRED)
find_package(Boost 1.58	COMPONENTS
            system filesystem)
find_package(Mongo_CXX_Driver)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -std=c++11 -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare -fno-omit-frame-pointer")



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})

set(Multiverso_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/multiverso/include)
set(LightLDA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/lightlda/include)

add_subdirectory(multiverso)
add_subdirectory(lightlda)
add_subdirectory(inference)
add_subdirectory(preprocess)


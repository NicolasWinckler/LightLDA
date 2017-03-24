# Set the following variables
# Mongo_CXX_Driver_FOUND
# Mongo_CXX_Driver_INCLUDE_DIR
# Mongo_CXX_Driver_LIBRARIES
# Mongo_CXX_Driver_LIBRARY_DIRS

# Mongo_Bson_CXX_INCLUDE_DIR
# Mongo_Bson_CXX_LIBRARIES


if(NOT Mongo_CXX_Driver_FIND_QUIETLY)
    message(STATUS "Looking for Mongo_CXX_Driver...")
endif()

# define header to search for
#set(Mongo_CXX_Driver_H "client.hpp")
set(Mongo_CXX_Driver_H    "mongocxx/client.hpp")
set(Mongo_Bson_CXX_H      "bsoncxx/json.hpp")

#define shared and static libs to search for
set(LIBMongo_CXX_Driver_SHARED libmongocxx.dylib libmongocxx.so)
set(LIBMongo_CXX_Driver_STATIC libmongocxx.a)

set(LIBMongo_Bson_CXX_SHARED libbsoncxx.so)
set(LIBMongo_Bson_CXX_STATIC libbsoncxx.a)


##################################################################
# default system directory search (see below), will be overwritten if
# - cmake USE_MONGO_CXX_DRIVER_PREFIX variable defined
# - MONGO_CXX_DRIVER_PREFIX environment variable defined
# - if both USE_MONGO_CXX_DRIVER_PREFIX and MONGO_CXX_DRIVER_PREFIX variables are defined, use the USE_MONGO_CXX_DRIVER_PREFIX
SET(SEARCH_DRIVER_HEADER_PATHS
        "/usr/local/mongo-cxx-driver-r3.1.1/include/mongocxx/v_noabi"
        )

SET(SEARCH_DRIVER_LIB_PATHS
        "/usr/local/mongo-cxx-driver-r3.1.1/lib"
        )

SET(SEARCH_BSON_HEADER_PATHS
        "/usr/local/mongo-cxx-driver-r3.1.1/include/bsoncxx/v_noabi"
        )

SET(SEARCH_BSON_LIB_PATHS
        "/usr/local/mongo-cxx-driver-r3.1.1/lib"
        )


# if MONGO_CXX_DRIVER_PREFIX env variable defined (and no cmake command), search in this directory
if(DEFINED ENV{MONGO_CXX_DRIVER_PREFIX} AND NOT USE_MONGO_CXX_DRIVER_PREFIX)
    set(MONGO_CXX_DRIVER_PREFIX "$ENV{MONGO_CXX_DRIVER_PREFIX}")
endif()

# if USE_MONGO_CXX_DRIVER_PREFIX cmake variable defined, search in this directory
if(USE_MONGO_CXX_DRIVER_PREFIX)
    set(MONGO_CXX_DRIVER_PREFIX "${USE_MONGO_CXX_DRIVER_PREFIX}")
endif()

# print warning if both, cmake and env var defined. print that we look only at the cmake var in this case
if(DEFINED ENV{MONGO_CXX_DRIVER_PREFIX} AND USE_MONGO_CXX_DRIVER_PREFIX)
    message(WARNING "Both the environement variable MONGO_CXX_DRIVER_PREFIX, and the cmake variable USE_MONGO_CXX_DRIVER_PREFIX are defined. Only the USE_MONGO_CXX_DRIVER_PREFIX will be used")
endif()

#####################################

if(DEFINED ENV{MONGO_CXX_DRIVER_PREFIX} OR USE_MONGO_CXX_DRIVER_PREFIX)
    # if cmake or env var defined, search only in the hint directory (using NO_DEFAULT_PATH)
    # and do not search in the system
    set(SEARCH_DRIVER_LIB_PATHS "${MONGO_CXX_DRIVER_PREFIX}/lib")
    set(SEARCH_DRIVER_HEADER_PATHS "${MONGO_CXX_DRIVER_PREFIX}/include/mongocxx/v_noabi")

    set(SEARCH_BSON_LIB_PATHS "${MONGO_CXX_DRIVER_PREFIX}/lib")
    set(SEARCH_BSON_HEADER_PATHS "${MONGO_CXX_DRIVER_PREFIX}/include/bsoncxx/v_noabi")

    find_path(Mongo_CXX_Driver_INCLUDE_DIR NAMES ${Mongo_CXX_Driver_H}
            PATHS ${SEARCH_DRIVER_HEADER_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to Mongo_CXX_Driver include header files."
            )

    find_library(Mongo_CXX_Driver_LIBRARY_SHARED NAMES ${LIBMongo_CXX_Driver_SHARED}
            PATHS ${SEARCH_DRIVER_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_CXX_Driver_SHARED}."
            )

    find_library(Mongo_CXX_Driver_LIBRARY_STATIC NAMES ${LIBMongo_CXX_Driver_STATIC}
            PATHS ${SEARCH_DRIVER_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_CXX_Driver_STATIC}."
            )

    ############### bsoncxx

    find_path(Mongo_Bson_CXX_INCLUDE_DIR NAMES ${Mongo_Bson_CXX_H}
            PATHS ${SEARCH_BSON_HEADER_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to Mongo_Bson_CXX include header files."
            )

    find_library(Mongo_Bson_CXX_LIBRARY_SHARED NAMES ${LIBMongo_Bson_CXX_SHARED}
            PATHS ${SEARCH_BSON_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_Bson_CXX_SHARED}."
            )

    find_library(Mongo_Bson_CXX_LIBRARY_STATIC NAMES ${LIBMongo_Bson_CXX_STATIC}
            PATHS ${SEARCH_BSON_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_Bson_CXX_STATIC}."
            )

else()
    # it no hint given, search in default system directory as defined above
    find_path(Mongo_CXX_Driver_INCLUDE_DIR NAMES ${Mongo_CXX_Driver_H}
            PATHS ${SEARCH_DRIVER_HEADER_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to Mongo_CXX_Driver include header files."
            )

    find_library(Mongo_CXX_Driver_LIBRARY_SHARED NAMES ${LIBMongo_CXX_Driver_SHARED}
            PATHS ${SEARCH_DRIVER_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_CXX_Driver_SHARED}."
            )

    find_library(Mongo_CXX_Driver_LIBRARY_STATIC NAMES ${LIBMongo_CXX_Driver_STATIC}
            PATHS ${SEARCH_DRIVER_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_CXX_Driver_STATIC}."
            )

    ############### bsoncxx

    find_path(Mongo_Bson_CXX_INCLUDE_DIR NAMES ${Mongo_Bson_CXX_H}
            PATHS ${SEARCH_BSON_HEADER_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to Mongo_Bson_CXX include header files."
            )

    find_library(Mongo_Bson_CXX_LIBRARY_SHARED NAMES ${LIBMongo_Bson_CXX_SHARED}
            PATHS ${SEARCH_BSON_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_Bson_CXX_SHARED}."
            )

    find_library(Mongo_Bson_CXX_LIBRARY_STATIC NAMES ${LIBMongo_Bson_CXX_STATIC}
            PATHS ${SEARCH_BSON_LIB_PATHS}
            NO_DEFAULT_PATH
            DOC   "Path to ${LIBMongo_Bson_CXX_STATIC}."
            )



endif()


# SET PACKAGE_FOUND var

# Mongo_CXX_Driver
set(ONLY_Mongo_CXX_Driver_FOUND FALSE)
#####################################

if(Mongo_CXX_Driver_LIBRARY_SHARED)
    set(Mongo_CXX_Driver_LIBRARIES "${Mongo_CXX_Driver_LIBRARY_SHARED}" ${Mongo_CXX_Driver_LIBRARIES})
endif()


#if(Mongo_CXX_Driver_LIBRARY_STATIC)
#  set(Mongo_CXX_Driver_LIBRARIES "${Mongo_CXX_Driver_LIBRARY_STATIC}" ${Mongo_CXX_Driver_LIBRARIES})
#endif()


if(Mongo_CXX_Driver_LIBRARY_SHARED AND Mongo_CXX_Driver_LIBRARY_STATIC)
    if(Mongo_CXX_Driver_INCLUDE_DIR)
        set(ONLY_Mongo_CXX_Driver_FOUND TRUE)
    endif()
endif()


# bsoncxx
set(ONLY_Mongo_Bson_CXX_FOUND FALSE)
#####################################

if(Mongo_Bson_CXX_LIBRARY_SHARED)
    set(Mongo_Bson_CXX_LIBRARIES "${Mongo_Bson_CXX_LIBRARY_SHARED}" ${Mongo_Bson_CXX_LIBRARIES})
endif()


#if(Mongo_Bson_CXX_LIBRARY_STATIC)
#  set(Mongo_Bson_CXX_LIBRARIES "${Mongo_Bson_CXX_LIBRARY_STATIC}" ${Mongo_Bson_CXX_LIBRARIES})
#endif()


if(Mongo_Bson_CXX_LIBRARY_SHARED AND Mongo_Bson_CXX_LIBRARY_STATIC)
    if(Mongo_Bson_CXX_INCLUDE_DIR)
        set(ONLY_Mongo_Bson_CXX_FOUND TRUE)
    endif()
endif()



#####################################
# check if mongocxx AND bsoncxx are there
if(ONLY_Mongo_CXX_Driver_FOUND AND ONLY_Mongo_Bson_CXX_FOUND)
    set(Mongo_CXX_Driver_FOUND TRUE)
endif(ONLY_Mongo_CXX_Driver_FOUND AND ONLY_Mongo_Bson_CXX_FOUND)






#####################################
if(Mongo_CXX_Driver_FOUND)
    set(Mongo_CXX_Driver_LIBRARY_DIRS ${SEARCH_DRIVER_LIB_PATHS} ${SEARCH_BSON_LIB_PATHS})
    if(NOT Mongo_CXX_Driver_FIND_QUIETLY)
        message(STATUS "Looking for Mongo_CXX_Driver... - found ${Mongo_CXX_Driver_LIBRARIES}")
        message(STATUS "Looking for Mongo_Bson_CXX_Driver... - found ${Mongo_Bson_CXX_LIBRARIES}")
        set(Mongo_CXX_Driver_LIBRARIES ${Mongo_CXX_Driver_LIBRARIES} ${Mongo_Bson_CXX_LIBRARIES})
        set(Mongo_CXX_Driver_INCLUDE_DIR ${Mongo_CXX_Driver_INCLUDE_DIR} ${Mongo_Bson_CXX_INCLUDE_DIR})
    endif(NOT Mongo_CXX_Driver_FIND_QUIETLY)
else(Mongo_CXX_Driver_FOUND)
    if(NOT Mongo_CXX_Driver_FIND_QUIETLY)
        if(Mongo_CXX_Driver_FIND_REQUIRED)
            message(FATAL_ERROR "Looking for Mongo_CXX_Driver... - Not found")
        else(Mongo_CXX_Driver_FIND_REQUIRED)
            message(STATUS "Looking for Mongo_CXX_Driver... - Not found")
        endif(Mongo_CXX_Driver_FIND_REQUIRED)
    endif(NOT Mongo_CXX_Driver_FIND_QUIETLY)
endif(Mongo_CXX_Driver_FOUND)

mark_as_advanced(Mongo_CXX_Driver_INCLUDE_DIR Mongo_CXX_Driver_LIBRARIES)

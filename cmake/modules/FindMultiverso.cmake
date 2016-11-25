


# Set the following variables
# Multiverso_FOUND
# Multiverso_INCLUDE_DIR
# Multiverso_LIBRARIES
# todo: Multiverso_BINARY --> do one for the LightLDA Multiverso?

message(STATUS "Looking for Multiverso...")

set(Multiverso_H multiverso.h)
set(LIBMultiverso_SHARED libmultiverso.dylib libmultiverso.so) # for new multiverso 
set(LIBMultiverso_STATIC libmultiverso_static.a) # for LightLDA with old multiverso 


##################################### where to search Multiverso?
SET(SEARCH_HEADER_PATHS 
    "/usr/local/include/multiverso"
    "/usr/include/multiverso"
    )

SET(SEARCH_LIB_PATHS 
	"/usr/local/lib"
    "/usr/lib/x86_64-linux-gnu"
    "/usr/lib"
    )



if(DEFINED ENV{MULTIVERSOPATH} AND NOT USE_MULTIVERSO_PATH)
  set(SEARCH_LIB_PATHS "$ENV{MULTIVERSOPATH}/lib")
  set(SEARCH_HEADER_PATHS "$ENV{MULTIVERSOPATH}/include/multiverso")
endif()

if(USE_MULTIVERSO_PATH)
  set(SEARCH_LIB_PATHS "${USE_MULTIVERSO_PATH}/lib")
  set(SEARCH_HEADER_PATHS "${USE_MULTIVERSO_PATH}/include/multiverso")
endif()

if(DEFINED ENV{MULTIVERSOPATH} AND USE_MULTIVERSO_PATH)
  message(WARNING "Both the environement variable MULTIVERSOPATH, and the cmake variable USE_MULTIVERSO_PATH are defined. Only the USE_MULTIVERSO_PATH will be used")
endif()

#####################################

find_path(Multiverso_INCLUDE_DIR NAMES ${Multiverso_H} 
  PATHS ${SEARCH_HEADER_PATHS}
  DOC   "Path to Multiverso include header files."
)

find_library(Multiverso_LIBRARY_SHARED NAMES ${LIBMultiverso_SHARED}
  PATHS ${SEARCH_LIB_PATHS}
  DOC   "Path to ${LIBMultiverso_SHARED}."
)

find_library(Multiverso_LIBRARY_STATIC NAMES ${LIBMultiverso_STATIC}
  PATHS ${SEARCH_LIB_PATHS}
  DOC   "Path to ${LIBMultiverso_STATIC}."
)


set(Multiverso_FOUND FALSE)
#####################################
if(Multiverso_LIBRARY_SHARED OR Multiverso_LIBRARY_STATIC) # OR or AND ?
  if(Multiverso_INCLUDE_DIR)
    set(Multiverso_FOUND TRUE)
  endif()
endif()


#####################################
if(Multiverso_FOUND)
  set(Multiverso_LIBRARIES "${Multiverso_LIBRARY_STATIC};${Multiverso_LIBRARY_SHARED}")
  if(NOT Multiverso_FIND_QUIETLY)
    message(STATUS "Looking for Multiverso... - found ${Multiverso_LIBRARIES}")
  endif(NOT Multiverso_FIND_QUIETLY)
else(Multiverso_FOUND)
  if(NOT Multiverso_FIND_QUIETLY)
    if(Multiverso_FIND_REQUIRED)
      message(FATAL_ERROR "Looking for Multiverso... - Not found")
    else(Multiverso_FIND_REQUIRED)
      message(STATUS "Looking for Multiverso... - Not found")
    endif(Multiverso_FIND_REQUIRED)
  endif(NOT Multiverso_FIND_QUIETLY)
endif(Multiverso_FOUND)

mark_as_advanced(Multiverso_INCLUDE_DIR Multiverso_LIBRARIES)

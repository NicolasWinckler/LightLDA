


# Set the following variables
# Multiverso_FOUND
# Multiverso_INCLUDE_DIR
# Multiverso_LIBRARIES
# todo: Multiverso_BINARY --> do one for the LightLDA Multiverso?

if(NOT Multiverso_FIND_QUIETLY)
	message(STATUS "Looking for Multiverso...")
endif()

# define header to search for
set(Multiverso_H "multiverso/multiverso.h")

#define shared and static libs to search for 
set(LIBMultiverso_SHARED libmultiverso.dylib libmultiverso.so) 
set(LIBMultiverso_STATIC libmultiverso.a)


##################################### where to search Multiverso?
# default system directory search, will be overwritten if
# - cmake USE_MULTIVERSO_PATH variable defined
# - MULTIVERSOPATH environment variable defined
# - if both USE_MULTIVERSO_PATH and MULTIVERSOPATH variables are defined, use the USE_MULTIVERSO_PATH
SET(SEARCH_HEADER_PATHS 
    "/usr/local/include"
    "/usr/include"
    )

SET(SEARCH_LIB_PATHS 
	"/usr/local/lib"
    "/usr/lib/x86_64-linux-gnu"
    "/usr/lib"
    )


# if MULTIVERSOPATH env variable defined (and no cmake command), search in this directory
if(DEFINED ENV{MULTIVERSOPATH} AND NOT USE_MULTIVERSO_PATH)
	set(MULTIVERSO_PREFIX "$ENV{MULTIVERSOPATH}")
endif()

# if USE_MULTIVERSO_PATH cmake variable defined, search in this directory
if(USE_MULTIVERSO_PATH)
	set(MULTIVERSO_PREFIX "${USE_MULTIVERSO_PATH}")
endif()

# print warning if both, cmake and env var defined. print that we look only at the cmake var in this case
if(DEFINED ENV{MULTIVERSOPATH} AND USE_MULTIVERSO_PATH)
	message(WARNING "Both the environement variable MULTIVERSOPATH, and the cmake variable USE_MULTIVERSO_PATH are defined. Only the USE_MULTIVERSO_PATH will be used")
endif()

#####################################

if(DEFINED ENV{MULTIVERSOPATH} OR USE_MULTIVERSO_PATH)
	# if cmake or env var defined, search only in the hint directory (using NO_DEFAULT_PATH) 
	# and do not search in the system 
	set(SEARCH_LIB_PATHS "${MULTIVERSO_PREFIX}/lib")
  set(SEARCH_HEADER_PATHS "${MULTIVERSO_PREFIX}/include")
	find_path(Multiverso_INCLUDE_DIR NAMES ${Multiverso_H} 
	  PATHS ${SEARCH_HEADER_PATHS}
	  NO_DEFAULT_PATH
	  DOC   "Path to Multiverso include header files."
	)

	find_library(Multiverso_LIBRARY_SHARED NAMES ${LIBMultiverso_SHARED}
	  PATHS ${SEARCH_LIB_PATHS}
	  NO_DEFAULT_PATH
	  DOC   "Path to ${LIBMultiverso_SHARED}."
	)

	find_library(Multiverso_LIBRARY_STATIC NAMES ${LIBMultiverso_STATIC}
	  PATHS ${SEARCH_LIB_PATHS}
	  NO_DEFAULT_PATH
	  DOC   "Path to ${LIBMultiverso_STATIC}."
	)
else()
	# it no hint given, search in default system directory as defined above
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
endif()

set(Multiverso_FOUND FALSE)
#####################################

if(Multiverso_LIBRARY_SHARED)
	set(Multiverso_LIBRARIES "${Multiverso_LIBRARY_SHARED}" ${Multiverso_LIBRARIES})
endif()


if(Multiverso_LIBRARY_STATIC)
	set(Multiverso_LIBRARIES "${Multiverso_LIBRARY_STATIC}" ${Multiverso_LIBRARIES})
endif()


if(Multiverso_LIBRARY_SHARED OR Multiverso_LIBRARY_STATIC) # OR because initial multiverso do not produce shared lib
	if(Multiverso_INCLUDE_DIR)
		set(Multiverso_FOUND TRUE)
	endif()
endif()


#####################################
if(Multiverso_FOUND)
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

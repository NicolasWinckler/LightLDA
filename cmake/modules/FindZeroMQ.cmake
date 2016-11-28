

SET(SEARCH_HEADER_PATHS 
    "/usr/local/include"
    "/usr/include"
    )

SET(SEARCH_LIB_PATHS 
    "/usr/lib/x86_64-linux-gnu"
    "/usr/lib"
    )

if(USE_ZMQ_PATH)
  SET(SEARCH_HEADER_PATHS ${USE_ZMQ_PATH}/include)
  SET(SEARCH_LIB_PATHS ${USE_ZMQ_PATH}/lib)
endif()

if(NOT ZeroMQ_FIND_QUIETLY)
  message(STATUS "Looking for ZeroMQ...")
endif(NOT ZeroMQ_FIND_QUIETLY)

if(USE_ZMQ_PATH)
  find_path(ZMQ_INCLUDE_DIR NAMES zmq.hpp zmq_utils.h
    PATHS ${SEARCH_HEADER_PATHS}
    NO_DEFAULT_PATH
    DOC   "Path to ZeroMQ include header files."
  )
else()
  find_path(ZMQ_INCLUDE_DIR NAMES zmq.hpp zmq_utils.h
    PATHS ${SEARCH_HEADER_PATHS}
    DOC   "Path to ZeroMQ include header files."
  )
endif()



if(USE_ZMQ_PATH)
  find_library(ZMQ_LIBRARY_SHARED NAMES libzmq.dylib libzmq.so
    PATHS ${SEARCH_LIB_PATHS}
    NO_DEFAULT_PATH
    DOC   "Path to libzmq.dylib libzmq.so."
  )
else()
  find_library(ZMQ_LIBRARY_SHARED NAMES libzmq.dylib libzmq.so
    PATHS ${SEARCH_LIB_PATHS}
    DOC   "Path to libzmq.dylib libzmq.so."
  )
endif()

if(USE_ZMQ_PATH)
  find_library(ZMQ_LIBRARY_STATIC NAMES libzmq.a
    PATHS ${USE_ZMQ_PATH}/lib
    NO_DEFAULT_PATH
    DOC   "Path to libzmq.a."
)
else()
  find_library(ZMQ_LIBRARY_STATIC NAMES libzmq.a
    PATHS ${SEARCH_LIB_PATHS}
    DOC   "Path to libzmq.a."
  )
endif()

IF(ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY_SHARED AND ZMQ_LIBRARY_STATIC)
  SET(ZMQ_FOUND TRUE)
ELSE(ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY_SHARED AND ZMQ_LIBRARY_STATIC)
  SET(ZMQ_FOUND FALSE)
ENDIF(ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY_SHARED AND ZMQ_LIBRARY_STATIC)

set(ERROR_STRING "Looking for ZeroMQ... - Not found")

if(ZMQ_FOUND)
  FIND_FILE(ZMQ_HEADER_FILE zmq.h
    ${ZMQ_INCLUDE_DIR}
    NO_DEFAULT_PATH
    )
  IF (DEFINED ZMQ_HEADER_FILE)
    FILE(READ "${ZMQ_HEADER_FILE}" _ZMQ_HEADER_FILE_CONTENT)
    STRING(REGEX MATCH "#define ZMQ_VERSION_MAJOR ([0-9])" _MATCH "${_ZMQ_HEADER_FILE_CONTENT}")
    SET(ZMQ_VERSION_MAJOR ${CMAKE_MATCH_1})
    STRING(REGEX MATCH "#define ZMQ_VERSION_MINOR ([0-9])" _MATCH "${_ZMQ_HEADER_FILE_CONTENT}")
    SET(ZMQ_VERSION_MINOR ${CMAKE_MATCH_1})
    STRING(REGEX MATCH "#define ZMQ_VERSION_PATCH ([0-9])" _MATCH "${_ZMQ_HEADER_FILE_CONTENT}")
    SET(ZMQ_VERSION_PATCH ${CMAKE_MATCH_1})
    set (ZMQ_VERSION "${ZMQ_VERSION_MAJOR}.${ZMQ_VERSION_MINOR}.${ZMQ_VERSION_PATCH}")
    IF (DEFINED ZeroMQ_FIND_VERSION AND ZMQ_VERSION VERSION_LESS ZeroMQ_FIND_VERSION)
      SET(ZMQ_FOUND FALSE)
      SET(ERROR_STRING "Installed version ${ZMQ_VERSION} of ZeroMQ does not meet the minimum required version ${ZeroMQ_FIND_VERSION}")
    endif ()
  endif ()
endif()

if(ZMQ_FOUND)
  set(ZMQ_LIBRARIES "${ZMQ_LIBRARY_STATIC};${ZMQ_LIBRARY_SHARED}")
  if(NOT ZeroMQ_FIND_QUIETLY)
    message(STATUS "Looking for ZeroMQ... - found ${ZMQ_LIBRARIES} ${ZMQ_VERSION}")
  endif(NOT ZeroMQ_FIND_QUIETLY)
else(ZMQ_FOUND)
  unset(ZMQ_INCLUDE_DIR)
  unset(ZMQ_LIBRARY_SHARED)
  unset(ZMQ_LIBRARY_STATIC)
  if(ZeroMQ_FIND_REQUIRED)
    message(FATAL_ERROR "${ERROR_STRING}")
  else(ZeroMQ_FIND_REQUIRED)
    if(NOT ZeroMQ_FIND_QUIETLY)
      message(STATUS "${ERROR_STRING}")
    endif(NOT ZeroMQ_FIND_QUIETLY)
  endif(ZeroMQ_FIND_REQUIRED)
endif(ZMQ_FOUND)

mark_as_advanced(ZMQ_INCLUDE_DIR ZMQ_LIBRARIES ZMQ_LIBRARY_SHARED ZMQ_LIBRARY_STATIC)

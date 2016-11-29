

set(HEADERS inferer.h)
set(SRCS inferer.cpp)

set(INCLUDE_DIRECTORIES 
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src
	${Multiverso_INCLUDE_DIR}
	)

set(DEPENDENCIES
	${ZMQ_LIBRARIES}
	${MPI_CXX_LIBRARIES}
	${Multiverso_LIBRARIES}
	pthread
	lightlda_base
)


####### Generate library
set(LIBRARY_NAME infer_base)

include_directories(${INCLUDE_DIRECTORIES})
Add_Library(${LIBRARY_NAME} SHARED ${SRCS} ${HEADERS})
target_link_libraries(${LIBRARY_NAME} ${DEPENDENCIES})

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)


####### Generate Executable
set(DEPENDENCIES infer_base)
set(SRCS infer.cpp)
set(EXE_NAME infer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} ${DEPENDENCIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)












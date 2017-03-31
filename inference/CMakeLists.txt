

set(HEADERS inferer.h)
set(SRCS inferer.cpp)

set(INCLUDE_DIRECTORIES 
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/dev/src
	${Multiverso_INCLUDE_DIR}
	)

set(DEPENDENCIES
	lightlda_core
	${Multiverso_LIBRARIES}
	pthread
	${ZMQ_LIBRARIES}
	${MPI_CXX_LIBRARIES}
)


####### Generate library
set(LIBRARY_NAME lightlda_infer)

include_directories(${INCLUDE_DIRECTORIES})
Add_Library(${LIBRARY_NAME} SHARED ${SRCS})
Add_Library(${LIBRARY_NAME}_static STATIC ${SRCS})
target_link_libraries(${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${LIBRARY_NAME}_static ${DEPENDENCIES})

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(TARGETS ${LIBRARY_NAME}_static DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)


####### Generate Executable
set(DEPENDENCIES lightlda_infer)
set(SRCS infer.cpp)
set(EXE_NAME infer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} ${DEPENDENCIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)












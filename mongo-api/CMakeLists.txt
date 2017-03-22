




set(INCLUDE_DIRECTORIES
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
	${Multiverso_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
    ${Mongo_CXX_Driver_INCLUDE_DIR}
	)


set(DEPENDENCIES
	#${Multiverso_LIBRARIES}#zmq runtime error if shared and static
	${Multiverso_LIBRARY_STATIC}
	pthread
	${ZMQ_LIBRARIES}
	${MPI_CXX_LIBRARIES}
    ${Mongo_CXX_Driver_LIBRARIES}
)




####### Generate lightlda Executable
set(SRCS run/mongoLightLda.cpp)
set(EXE_NAME mongo-lda)

include_directories(${INCLUDE_DIRECTORIES})
Add_Executable(${EXE_NAME} ${SRCS})

#target_link_libraries(${EXE_NAME} ${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${EXE_NAME} lightlda_core_static ${DEPENDENCIES} ${Multiverso_LIBRARY_STATIC} ${Boost_LIBRARIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)



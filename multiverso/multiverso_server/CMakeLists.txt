
set(INCLUDE_DIRECTORIES
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${Multiverso_INCLUDE_DIR}
	)


set(DEPENDENCIES mpich mpl zmq pthread multiverso)

if(Mongo_CXX_Driver_FOUND)

	set(INCLUDE_DIRECTORIES
			${Mongo_CXX_Driver_INCLUDE_DIR}
			${CMAKE_SOURCE_DIR}/mongo/include
			${INCLUDE_DIRECTORIES}
			)

	set(DEPENDENCIES
            ${Mongo_CXX_Driver_LIBRARIES}
            ${DEPENDENCIES}
			)
endif()

include_directories(${INCLUDE_DIRECTORIES})

SET(MULTIVERSO_SERVER_SRC cl_parser.cpp main.cpp)


ADD_EXECUTABLE(multiverso.server ${MULTIVERSO_SERVER_SRC})

TARGET_LINK_LIBRARIES(multiverso.server ${DEPENDENCIES})

SET_PROPERTY(TARGET multiverso.server PROPERTY CXX_STANDARD 11)

install(TARGETS multiverso.server DESTINATION bin)

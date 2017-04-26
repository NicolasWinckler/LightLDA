
INCLUDE_DIRECTORIES(
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_SOURCE_DIR}/include/multiverso
	)

SET(MULTIVERSO_SERVER_SRC cl_parser.cpp main.cpp)


ADD_EXECUTABLE(multiverso.server ${MULTIVERSO_SERVER_SRC})

TARGET_LINK_LIBRARIES(multiverso.server mpich mpl zmq pthread multiverso)

SET_PROPERTY(TARGET multiverso.server PROPERTY CXX_STANDARD 11)

install(TARGETS multiverso.server DESTINATION bin)

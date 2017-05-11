set(INCLUDE_DIRECTORIES
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_SOURCE_DIR}/include/multiverso
	)

set(DEPENDENCIES mpich mpl zmq pthread)

if(Mongo_CXX_Driver_FOUND)
	set(INCLUDE_DIRECTORIES
		${Mongo_CXX_Driver_INCLUDE_DIR}
		${CMAKE_SOURCE_DIR}/include/mongo-api
		${INCLUDE_DIRECTORIES})
	set(DEPENDENCIES
            ${Mongo_CXX_Driver_LIBRARIES}
            ${DEPENDENCIES})
endif()


include_directories(${INCLUDE_DIRECTORIES})


SET(MULTIVERSO_SRC 
	multiverso.cpp 
	aggregator.cpp
	barrier.cpp
	communicator.cpp
	data_block.cpp
	delta_pool.cpp
	endpoint_list.cpp
	lock.cpp
	log.cpp
	mpi_util.cpp
	msg_pack.cpp
	parameter_loader.cpp
	row.cpp
	row_iter.cpp
	server.cpp
	stop_watch.cpp
	table.cpp
	table_iter.cpp
	trainer.cpp
	vector_clock.cpp
	zmq_util.cpp
	)


SET(MULTIVERSO_HDR 
	${CMAKE_SOURCE_DIR}/include/multiverso/multiverso.h
	${CMAKE_SOURCE_DIR}/include/multiverso/aggregator.h
	${CMAKE_SOURCE_DIR}/include/multiverso/barrier.h
	${CMAKE_SOURCE_DIR}/include/multiverso/communicator.h
	${CMAKE_SOURCE_DIR}/include/multiverso/data_block.h
	${CMAKE_SOURCE_DIR}/include/multiverso/delta_pool.h
	${CMAKE_SOURCE_DIR}/include/multiverso/double_buffer.h
	${CMAKE_SOURCE_DIR}/include/multiverso/endpoint_list.h
	${CMAKE_SOURCE_DIR}/include/multiverso/lock.h
	${CMAKE_SOURCE_DIR}/include/multiverso/log.h
	${CMAKE_SOURCE_DIR}/include/multiverso/meta.h
	${CMAKE_SOURCE_DIR}/include/multiverso/mpi_util.h
	${CMAKE_SOURCE_DIR}/include/multiverso/msg_pack.h
	${CMAKE_SOURCE_DIR}/include/multiverso/mt_queue.h
	${CMAKE_SOURCE_DIR}/include/multiverso/parameter_loader.h
	${CMAKE_SOURCE_DIR}/include/multiverso/row.h
	${CMAKE_SOURCE_DIR}/include/multiverso/row_iter.h
	${CMAKE_SOURCE_DIR}/include/multiverso/server.h
	${CMAKE_SOURCE_DIR}/include/multiverso/stop_watch.h
	${CMAKE_SOURCE_DIR}/include/multiverso/table.h
	${CMAKE_SOURCE_DIR}/include/multiverso/table_iter.h
	${CMAKE_SOURCE_DIR}/include/multiverso/trainer.h
	${CMAKE_SOURCE_DIR}/include/multiverso/vector_clock.h
	${CMAKE_SOURCE_DIR}/include/multiverso/zmq_util.h
	)

#ADD_LIBRARY(multiverso ${MULTIVERSO_SRC})
Add_Library(multiverso SHARED ${MULTIVERSO_SRC})
Add_Library(multiverso_static STATIC ${MULTIVERSO_SRC})

TARGET_LINK_LIBRARIES(multiverso ${DEPENDENCIES})
TARGET_LINK_LIBRARIES(multiverso_static ${DEPENDENCIES})

SET_PROPERTY(TARGET multiverso PROPERTY CXX_STANDARD 11)



install(TARGETS multiverso DESTINATION lib)
install(TARGETS multiverso_static DESTINATION lib)
install(FILES ${MULTIVERSO_HDR} DESTINATION include/multiverso)



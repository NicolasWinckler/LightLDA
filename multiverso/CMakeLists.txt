set(INCLUDE_DIRECTORIES
        ${ZMQ_INCLUDE_DIR}
        ${MPI_CXX_INCLUDE_PATH}
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso
        )

set(DEPENDENCIES mpich mpl zmq pthread)

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


SET(MULTIVERSO_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/multiverso.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/aggregator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/barrier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/communicator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/data_block.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/delta_pool.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/endpoint_list.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/mpi_util.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/msg_pack.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parameter_loader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/row.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/row_iter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/stop_watch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/table.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/table_iter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/trainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vector_clock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/zmq_util.cpp
        )

SET(MULTIVERSO_HDR
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/multiverso.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/aggregator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/barrier.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/communicator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/data_block.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/delta_pool.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/double_buffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/endpoint_list.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/lock.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/log.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/meta.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/mpi_util.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/msg_pack.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/mt_queue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/parameter_loader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/row.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/row_iter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/server.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/stop_watch.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/table.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/table_iter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/trainer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/vector_clock.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/multiverso/zmq_util.h
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


add_subdirectory(multiverso_server)



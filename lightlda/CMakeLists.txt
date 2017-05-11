
set(HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/alias_table.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/data_block.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/data_stream.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/document.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/eval.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/meta.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/model.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sampler.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/trainer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/util.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Vocab.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/qxlda.hpp
        )

set(SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/alias_table.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/src/data_block.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/data_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/document.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/eval.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/src/meta.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sampler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/trainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Vocab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/qxlda.cpp
        )


set(INCLUDE_DIRECTORIES
        ${ZMQ_INCLUDE_DIR}
        ${MPI_CXX_INCLUDE_PATH}
        ${Multiverso_INCLUDE_DIR}
        ${Boost_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

# Note: order of the dependencies matter for linking with static libs
set(DEPENDENCIES
        #${Multiverso_LIBRARIES}#zmq runtime error if shared and static
        #${Multiverso_LIBRARY_STATIC}
        multiverso_static
        pthread
        ${ZMQ_LIBRARIES}
        ${MPI_CXX_LIBRARIES}
        )

if(Mongo_CXX_Driver_FOUND)
    set(INCLUDE_DIRECTORIES
            ${Mongo_CXX_Driver_INCLUDE_DIR}
            ${CMAKE_SOURCE_DIR}/mongo/include
            ${INCLUDE_DIRECTORIES})
    set(DEPENDENCIES ${Mongo_CXX_Driver_LIBRARIES} ${DEPENDENCIES})
endif()

####### Generate BaseLightLDA library
set(LIBRARY_NAME lightlda_core)

include_directories(${INCLUDE_DIRECTORIES})
Add_Library(${LIBRARY_NAME} SHARED ${SRCS})
Add_Library(${LIBRARY_NAME}_static STATIC ${SRCS})
target_link_libraries(${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${LIBRARY_NAME}_static ${DEPENDENCIES})


install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(TARGETS ${LIBRARY_NAME}_static DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/lightlda)



####### Generate lightlda Executable
set(SRCS src/lightlda.cpp)
set(EXE_NAME lightlda)

include_directories(${INCLUDE_DIRECTORIES})
Add_Executable(${EXE_NAME} ${SRCS})

#target_link_libraries(${EXE_NAME} ${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${EXE_NAME} ${LIBRARY_NAME}_static ${DEPENDENCIES}
        multiverso_static
        #${Multiverso_LIBRARY_STATIC}
        )

install(TARGETS ${EXE_NAME} DESTINATION bin)



####### Generate lightlda Executable (with mongoDB I/O)
set(SRCS src/runqxlda.cpp)
set(EXE_NAME runlda_mongo)

include_directories(${INCLUDE_DIRECTORIES})
Add_Executable(${EXE_NAME} ${SRCS})

#target_link_libraries(${EXE_NAME} ${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${EXE_NAME} ${LIBRARY_NAME}_static ${DEPENDENCIES}
        #${Multiverso_LIBRARY_STATIC}
        multiverso_static
        ${Boost_LIBRARIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)





####### Generate lightlda Executable
#[[set(SRCS mongotest.cpp)
set(EXE_NAME mongotest)

include_directories(${INCLUDE_DIRECTORIES})
Add_Executable(${EXE_NAME} ${SRCS})

#target_link_libraries(${EXE_NAME} ${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${EXE_NAME} ${LIBRARY_NAME}_static ${DEPENDENCIES} ${Multiverso_LIBRARY_STATIC} ${Boost_LIBRARIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)]]







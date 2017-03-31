
set(HEADERS 
	alias_table.h
	common.h
	data_block.h
	data_stream.h
	document.h
	eval.h
	meta.h
	model.h
	sampler.h
	trainer.h
	util.h
	Vocab.h
	)

set(SRCS 
	alias_table.cpp
	common.cpp
	#data_block.cpp
	data_stream.cpp
	document.cpp
	eval.cpp
	#meta.cpp
	model.cpp
	sampler.cpp
	trainer.cpp
	Vocab.cpp
	)


set(INCLUDE_DIRECTORIES 
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Multiverso_INCLUDE_DIR}
	${Boost_INCLUDE_DIR}
	)

# Note: order of the dependencies matter for linking with static libs
set(DEPENDENCIES
	#${Multiverso_LIBRARIES}#zmq runtime error if shared and static
	${Multiverso_LIBRARY_STATIC}
	pthread
	${ZMQ_LIBRARIES}
	${MPI_CXX_LIBRARIES}
)

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
set(SRCS lightlda.cpp)
set(EXE_NAME lightlda)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})

#target_link_libraries(${EXE_NAME} ${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${EXE_NAME} ${LIBRARY_NAME}_static ${DEPENDENCIES} ${Multiverso_LIBRARY_STATIC})

install(TARGETS ${EXE_NAME} DESTINATION bin)



####### Generate lightlda Executable
set(SRCS qxlda.cpp)
set(EXE_NAME qxlda)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})

#target_link_libraries(${EXE_NAME} ${LIBRARY_NAME} ${DEPENDENCIES})
target_link_libraries(${EXE_NAME} ${LIBRARY_NAME}_static ${DEPENDENCIES} ${Multiverso_LIBRARY_STATIC} ${Boost_LIBRARIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)












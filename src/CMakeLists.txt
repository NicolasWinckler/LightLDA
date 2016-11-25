





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
	)

set(SRCS 
	alias_table.cpp
	common.cpp
	data_block.cpp
	data_stream.cpp
	document.cpp
	eval.cpp
	meta.cpp
	model.cpp
	sampler.cpp
	trainer.cpp
	)


set(INCLUDE_DIRECTORIES 
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Multiverso_INCLUDE_DIR}/..
	)



set(DEPENDENCIES
	#${Boost_LIBRARIES}
	${ZMQ_LIBRARIES}
	${MPI_CXX_LIBRARIES}
	${Multiverso_LIBRARIES}
	#mpich 
	#mpl 
	pthread
)

message(STATUS "Multiverso_INCLUDE_DIR ----------> ${Multiverso_INCLUDE_DIR}")
message(STATUS "Multiverso_LIBRARIES ----------> ${Multiverso_LIBRARIES}")

#######

set(LIBRARY_NAME lightlda_base)

include_directories(${INCLUDE_DIRECTORIES})

Add_Library(${LIBRARY_NAME} SHARED ${SRCS} ${HEADERS})
target_link_libraries(${LIBRARY_NAME} ${DEPENDENCIES})

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)
#GENERATE_LIBRARY()












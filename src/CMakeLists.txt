
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
	${Multiverso_INCLUDE_DIR}
	)

set(DEPENDENCIES
	${ZMQ_LIBRARIES}
	${MPI_CXX_LIBRARIES}
	${Multiverso_LIBRARIES}
	pthread
)

####### Generate lightlda_base library
set(LIBRARY_NAME lightlda_base)

include_directories(${INCLUDE_DIRECTORIES})
Add_Library(${LIBRARY_NAME} SHARED ${SRCS} ${HEADERS})
target_link_libraries(${LIBRARY_NAME} ${DEPENDENCIES})

install(TARGETS ${LIBRARY_NAME} DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)



####### Generate lightlda Executable
set(DEPENDENCIES lightlda_base)
set(SRCS lightlda.cpp)
set(EXE_NAME lightlda)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} lightlda_base)

install(TARGETS ${EXE_NAME} DESTINATION bin)












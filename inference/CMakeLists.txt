

set(HEADERS
        include/inferer.h
        include/inferWrapper.hpp
        )
set(SRCS
        src/inferer.cpp
        src/inferWrapper.cpp
        )

set(INCLUDE_DIRECTORIES 
	${ZMQ_INCLUDE_DIR}
	${MPI_CXX_INCLUDE_PATH}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${LightLDA_INCLUDE_DIR}
	${Multiverso_INCLUDE_DIR}
	)

set(DEPENDENCIES
	lightlda_core
	#${Multiverso_LIBRARIES}
	multiverso_static
	pthread
	${ZMQ_LIBRARIES}
	${MPI_CXX_LIBRARIES}
)

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


####### Generate Executable (old)
set(DEPENDENCIES lightlda_infer)
set(SRCS run/infer.cpp)
set(EXE_NAME infer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} ${DEPENDENCIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)

####### Generate Executable
set(DEPENDENCIES lightlda_infer)
set(SRCS run/inferNewDocument.cpp)
set(EXE_NAME inferNewDoc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME} ${DEPENDENCIES})

install(TARGETS ${EXE_NAME} DESTINATION bin)











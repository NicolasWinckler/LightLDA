


set(SRCS dump_binary.cpp)
set(EXE_NAME dump_binary)

#######


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME})
install(TARGETS ${EXE_NAME} DESTINATION bin)






if(Mongo_CXX_Driver_FOUND)

    set(SRCS dump_mongo.cpp)
    set(EXE_NAME dump_mongo)

    #######

    set(INCLUDE_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/src/lightlda
            ${Mongo_CXX_Driver_INCLUDE_DIR}
            )

    set(DEPENDENCIES ${Mongo_CXX_Driver_LIBRARIES})

    include_directories(${INCLUDE_DIRECTORY})
    Add_Executable(${EXE_NAME} ${SRCS})
    target_link_libraries(${EXE_NAME} ${DEPENDENCIES})
    install(TARGETS ${EXE_NAME} DESTINATION bin)

endif()


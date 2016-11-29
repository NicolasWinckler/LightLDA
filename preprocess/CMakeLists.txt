


set(SRCS dump_binary.cpp)
set(EXE_NAME dump_binary)

#######


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
Add_Executable(${EXE_NAME} ${SRCS})
target_link_libraries(${EXE_NAME})
install(TARGETS ${EXE_NAME} DESTINATION bin)








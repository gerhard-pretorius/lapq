
#-----------------------------------------------------------------------------
file(GLOB SRC_FILES *.cpp)

add_executable(t1 t1.cpp)
add_executable(t2 t2.cpp)
add_executable(t3 t3.cpp)
add_executable(t4 t4.cpp)
add_executable(t5 t5.cpp)
add_executable(enum enum.cpp)
add_executable(numeric numeric.cpp)
add_executable(money money.cpp)

target_link_libraries(t1 ${LIB} ${REQ_LIBRARIES})
target_link_libraries(t2 ${LIB} ${REQ_LIBRARIES})
target_link_libraries(t3 ${LIB} ${REQ_LIBRARIES})
target_link_libraries(t4 ${LIB} ${REQ_LIBRARIES})
target_link_libraries(t5 ${LIB} ${REQ_LIBRARIES})
target_link_libraries(enum ${LIB} ${REQ_LIBRARIES})
target_link_libraries(numeric ${LIB} ${REQ_LIBRARIES})
target_link_libraries(money ${LIB} ${REQ_LIBRARIES})


set(TGT utest)

set(HFILES
  function-runner.h
)

set(CFILES
  function-runner.cpp
)


add_library(${TGT} STATIC ${CFILES})

target_include_directories(${TGT} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS ${TGT} DESTINATION lib)
install(FILES ${HFILES} DESTINATION include)



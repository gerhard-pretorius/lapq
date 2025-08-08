
#-----------------------------------------------------------------------------
set(LIBS ${LIBLAPQ} utest ${REQ_LIBRARIES})

#-----------------------------------------------------------------------------
# Adds an executable with a name that matches the src filename without
# the extension
#
function(AddExec fname)
  set(name "")
  get_filename_component(name "${fname}" NAME_WE)
  add_executable(${name} ${fname})
  target_link_libraries(${name} ${LIBS})
endfunction()

#-----------------------------------------------------------------------------
file(GLOB SRC_FILES *.cpp)


AddExec(connect.cpp)
AddExec(t1.cpp)
AddExec(t2.cpp)
AddExec(t3.cpp)
AddExec(t4.cpp)
AddExec(t5.cpp)
AddExec(enum.cpp)
AddExec(numeric.cpp)
AddExec(money.cpp)


#-----------------------------------------------------------------------------
# ctest tests
add_subdirectory(cmake)

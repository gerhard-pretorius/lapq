
#-----------------------------------------------------------------------------
# Unittest: Adds a test called name that runs a command and compares the result
# against pass or fail expressions.
#
function(UTest name command pass fail)
  add_test(NAME ${name} COMMAND ${command} ${ARGN})
  set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION "${pass}")
  set_tests_properties(${name} PROPERTIES FAIL_REGULAR_EXPRESSION "${fail}")
endfunction()



#-----------------------------------------------------------------------------

set(err "[Ee]rror")
set(ok "[Oo]k")
set(empty "^0 \"\"\n")

#-----------------------------------------------------------------------------
set(SRC_FILES
  connect.ctest
)

include(${SRC_FILES})


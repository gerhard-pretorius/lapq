
#-----------------------------------------------------------------------------
#file(GLOB SRC_FILES *.cpp *.h)

set(SRC_FILES
    misc.cpp
    error.cpp
    util.cpp
    pgformat.cpp
    protocol.cpp
    dbquery.cpp
    dbresult.cpp
    connection.cpp
    fsm.cpp
    dbconnection.cpp
    )

set(HDR_FILES
    misc.h
    error.h
    util.h
    pgtype.h
    pgformat.h
    protocol.h
    dbquery.h
    dbresult.h
    connection.h
    fsm.h
    dbconnection.h
    )


add_library(${LIB} SHARED ${SRC_FILES} ${HDR_FILES})
set_property(TARGET ${LIB} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_target_properties(${LIB} PROPERTIES VERSION ${LIB_VERSION} SOVERSION ${LIB_MAJOR})


#-----------------------------------------------------------------------------
# Exported header files

set(EXP_FILES
    lapq.h
)

install(FILES ${EXP_FILES} DESTINATION include)
install(TARGETS ${LIB} DESTINATION lib COMPONENT lib)


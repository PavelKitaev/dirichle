get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)

if ( USE_TBB )
    set(ProjectId "${ProjectId}_tbb")
    project( ${ProjectId} )
    message( STATUS "-- " ${ProjectId} )

    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h)

    set(PACK_LIB "${ProjectId}_lib")
    add_library(${PACK_LIB} STATIC ${ALL_SOURCE_FILES} )

    add_executable( ${ProjectId} ${ALL_SOURCE_FILES} )

    target_link_libraries(${ProjectId} ${PACK_LIB})
    if( WIN32 )
        target_link_libraries(${ProjectId} ${TBB_IMPORTED_TARGETS})
    else( WIN32 )
        target_link_libraries(${ProjectId} ${TBB_LIBRARIES})
    endif( WIN32 )

else( USE_TBB )
    message( STATUS "-- ${ProjectId} - NOT BUILD!"  )
endif( USE_TBB )

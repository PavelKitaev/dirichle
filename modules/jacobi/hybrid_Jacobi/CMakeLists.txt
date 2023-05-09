get_filename_component(ProjectId ${CMAKE_CURRENT_SOURCE_DIR} NAME)

if( USE_MPI AND USE_OMP )
    if( UNIX )
        set(CMAKE_C_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-uninitialized")
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-uninitialized")
    endif( UNIX )

    set(ProjectId "${ProjectId}_hybrid")
    project( ${ProjectId} )
    message( STATUS "-- " ${ProjectId} )

    file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h)

    set(PACK_LIB "${ProjectId}_mpi_lib")
    add_library(${PACK_LIB} STATIC ${ALL_SOURCE_FILES} )

    set(PACK_LIB_omp "${ProjectId}_omp_lib")
    add_library(${PACK_LIB_omp} STATIC ${ALL_SOURCE_FILES} )

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

    add_executable( ${ProjectId} ${ALL_SOURCE_FILES} )

    target_link_libraries(${ProjectId} ${PACK_LIB})
    target_link_libraries(${ProjectId} ${PACK_LIB_omp})

    if(OpenMP_CXX_FOUND)
        target_link_libraries(${ProjectId} OpenMP::OpenMP_CXX)
    else( )
       message( STATUS "-- ${ProjectId} - NOT OpenMP!"  )
    endif()

    if( MPI_COMPILE_FLAGS )
        set_target_properties( ${ProjectId} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}" )
    endif( MPI_COMPILE_FLAGS )

    if( MPI_LINK_FLAGS )
        set_target_properties( ${ProjectId} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}" )
    endif( MPI_LINK_FLAGS )

    target_link_libraries( ${ProjectId} ${MPI_LIBRARIES} )

else( USE_MPI AND USE_OMP )
    message( STATUS "-- ${ProjectId} - NOT BUILD!"  )
endif( USE_MPI AND USE_OMP )

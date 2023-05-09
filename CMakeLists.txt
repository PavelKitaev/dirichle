cmake_minimum_required( VERSION 3.2 )

project(diplom)
message( STATUS "Solution of the Dirichlet problem for the Poisson equation" )
include( "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Configure.cmake" )

################################ MPI ################################
option(USE_MPI ON)
if( USE_MPI )
    find_package( MPI )
    if( MPI_FOUND )
        include_directories( ${MPI_INCLUDE_PATH} )
    else( MPI_FOUND )
        set( USE_MPI OFF )
    endif( MPI_FOUND )
endif( USE_MPI )

############################### OpenMP ##############################
option(USE_OMP ON)
if( USE_OMP OR USE_SEQ )

    find_package(OpenMP REQUIRED)

    if( OpenMP_CXX_FOUND )
        message(STATUS  "OpenMP_CXX_FOUND DETECTED!")
        include_directories( ${OpenMP__CXX_INCLUDE_DIRS} )
    else( OpenMP_CXX_FOUND )
	message(STATUS  "OpenMP_CXX_FOUND NOT DETECTED!")
        set( USE_OMP OFF )
    endif( OpenMP_CXX_FOUND )
endif( USE_OMP OR USE_SEQ )

############################ std::thread ############################
option(USE_STD ON)
if( USE_STD )
    find_package( Threads )
endif( USE_STD )

################################ TBB ################################
option(USE_TBB ON)
if( USE_TBB )
    if( WIN32 )
        include( cmake/TBBGet.cmake )
        tbb_get( TBB_ROOT tbb_root RELEASE_TAG "v2021.8" CONFIG_DIR TBB_DIR )

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SYSTEM_BIT "intel64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(SYSTEM_BIT "ia32")
        endif()

        file( GLOB TBB_PRE_LIB  "${tbb_root}/bin/${SYSTEM_BIT}/vc14/*.dll" )
        file( COPY ${TBB_PRE_LIB} DESTINATION "${CMAKE_BINARY_DIR}/bin")

        find_package( TBB )
    else( WIN32 )
        find_package(TBB 2021.8.0 REQUIRED)
    endif( WIN32 )

    if( TBB_FOUND )
        message(STATUS "Found TBB: TRUE (found version \"${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR}\") ")
        include_directories("${tbb_root}/include")
    else( TBB_FOUND )
        set( USE_TBB OFF )
    endif( TBB_FOUND )
endif( USE_TBB )

############################## Modules ##############################
include_directories(3rdparty/unapproved)
add_subdirectory(modules)
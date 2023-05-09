message(STATUS "Gauss-Jacobi")

SUBDIRLIST(subdirs ${CMAKE_CURRENT_SOURCE_DIR})

foreach(subd ${subdirs})
  add_subdirectory(${subd})
endforeach()

add_library(ProjectOptions INTERFACE)
target_compile_features(ProjectOptions INTERFACE cxx_std_17)

if(MSVC)
# TODO(seidl):
# move this somewhere else
  target_compile_options(ProjectOptions INTERFACE -D_CRT_SECURE_NO_WARNINGS)
  foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR} )
  endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
else()
  if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OPTIMIZATION_LEVEL -Ofast)
  else()
    set(OPTIMIZATION_LEVEL -O0)
  endif() # If Release build type
  
  target_compile_options(ProjectOptions INTERFACE
    -Wno-reorder
    -pedantic-errors
    ${OPTIMIZATION_LEVEL}
    -fno-fast-math)
endif()
# TODO(seidl):
#add stricter warnings and linting

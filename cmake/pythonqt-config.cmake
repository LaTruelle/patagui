# Find PythonQt
#
# Sets PYTHONQT_FOUND, PYTHONQT_INCLUDE_DIR, PYTHONQT_LIBRARY, PYTHONQT_LIBRARIES
#

include(ExternalProject)

# Extract PythonQt from modified sources, then build
ExternalProject_Add(PythonQt-External
PREFIX pythonqt
# Backup URL in case SVN messes up.
URL "${CMAKE_CURRENT_SOURCE_DIR}/external_libs/pythonqt.zip"
CMAKE_ARGS -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
BUILD_IN_SOURCE 1
INSTALL_COMMAND ""
# Wrap download, configure and build steps in a script to log output
LOG_DOWNLOAD ON
LOG_CONFIGURE ON
LOG_BUILD ON
)

set(PYTHONQT_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythonqt/src/PythonQt-External)
set(PYTHONQT_INCLUDE_DIR ${PYTHONQT_INSTALL_DIR}/src)
set(PYTHONQT_LIBRARY PythonQt)

# Create dummy PythonQtLib that depends on real pythonqt in order to have cmake behaving properly without having to resort
# to a superbuild mechanism.
add_library(PythonQt SHARED IMPORTED)
set_target_properties(PythonQt PROPERTIES IMPORTED_LOCATION ${PYTHONQT_INSTALL_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}PythonQt${CMAKE_SHARED_LIBRARY_SUFFIX})
add_dependencies(PythonQt PythonQt-External)

# Used for OSX deployment.
get_target_property(PYTHONQT_LOCATION PythonQt LOCATION)

mark_as_advanced(PYTHONQT_INSTALL_DIR)
mark_as_advanced(PYTHONQT_INCLUDE_DIR)

set(PYTHONQT_FOUND 0)
if(PYTHONQT_INCLUDE_DIR AND PYTHONQT_LIBRARY)
    # Currently CMake'ified PythonQt only supports building against a python Release build.
    # This applies independently of CTK build type (Release, Debug, ...)
    add_definitions(-DPYTHONQT_USE_RELEASE_PYTHON_FALLBACK)
    set(PYTHONQT_LIBRARIES ${PYTHONQT_LIBRARY})
    set(PYTHONQT_FOUND 1)
endif()

if(NOT ${PYTHONQT_FOUND})
  message(FATAL_ERROR "PythonQt is required to build ${PROJECT_NAME}")
else()
  message(STATUS "PythonQt found")
endif()

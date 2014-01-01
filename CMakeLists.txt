project (Preferans)
cmake_minimum_required (VERSION 2.6)

# project version
enable_language(CXX)
set (Preferans_VERSION_MAJOR 0)
set (Preferans_VERSION_MINOR 01)
set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (RUNTIME_OUTPUT_DIRECTORY $(PROJECT_SOURCE_DIR))

configure_file (
"${PROJECT_SOURCE_DIR}/PreferansConfig.h.in"
"${PROJECT_SOURCE_DIR}/include/PreferansConfig.h"
)

find_package(Boost 1.36.0 REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/include")

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-O2 -DLOG_TO_STDOUT") #Optimization
endif()

add_subdirectory ("${PROJECT_SOURCE_DIR}/src/core")
add_subdirectory ("${PROJECT_SOURCE_DIR}/src/gui/gtkmm")

install (DIRECTORY PreferansData DESTINATION share/PreferansData)


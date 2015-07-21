cmake_minimum_required(VERSION 3.2)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include (MacroOpenSplice)
find_package (OpenSplice REQUIRED)

#
# You may add your include directories below
#
include_directories(
  ${PROJECT_SOURCE_DIR/src}
  ${OpenSplice_INCLUDE_DIRS}
)

file (GLOB_RECURSE DILib_SOURCES ./dilib/*.cxx)

set (CMAKE_CXX_FLAGS "-g -O2 -std=c++11 -Wall")

# library setup
add_library(lib${PROJECT_NAME}  ${DILib_SOURCES} ${OpenSplice_LIBRARIES})
target_link_libraries (lib${PROJECT_NAME} ${OpenSplice_LIBRARIES})

add_custom_target( lib DEPENDS lib${PROJECT_NAME} )

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/dilib" DESTINATION include PATTERN "*.hxx")

install( TARGETS lib${PROJECT_NAME} DESTINATION lib PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE)

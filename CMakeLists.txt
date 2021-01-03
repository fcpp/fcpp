# CMake 3.9 or newer
cmake_minimum_required(VERSION 3.9)

# C++14 or higher
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

# adding fcpp
add_subdirectory(./src/)

# project name, version and description
project(fcppGL
    VERSION 0.1.0
    DESCRIPTION "FieldCalc++ OpenGL simulator."
)

# executable target
add_executable(fcppGL ./src/test/simulation/displayer.cpp)

# VERSION property
set_target_properties(fcppGL PROPERTIES VERSION &{PROJECT_VERSION})

# link libraries
target_link_libraries(fcppGL PRIVATE fcpp)

#copying shaders into build
add_custom_command(TARGET fcppGL PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/src/lib/graphics/shaders/ $<TARGET_FILE_DIR:fcppGL>/shaders/)

#copying fonts into build
add_custom_command(TARGET fcppGL PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/src/lib/graphics/fonts/ $<TARGET_FILE_DIR:fcppGL>/fonts/)

#target compile options
target_compile_options(fcppGL PUBLIC "-DFCPP_CONFIGURATION=FCPP_CONFIGURATION_DEPENDENT")
#target_compile_options(fcppGL PUBLIC "-DPROFILER")

# defining install rules
include(GNUInstallDirs)
install(TARGETS fcppGL
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

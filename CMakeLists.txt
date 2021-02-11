# CMake 3.9 or newer
cmake_minimum_required(VERSION 3.9)

# Compiling options
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
if(WIN32)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")
endif(WIN32)

# Adding fcpp
option(FCPP_BUILD_GL "Build OpenGL features." ON) # this tells CMake to compile fcpp's OpenGL features
add_subdirectory(./src/)

# Project name, version and description
project(fcppGL
    VERSION 0.1.0
    DESCRIPTION "FieldCalc++ OpenGL simulator."
)

# Executable target
add_executable(fcppGL ./src/test/simulation/displayer.cpp)

# VERSION property
set_target_properties(fcppGL PROPERTIES VERSION &{PROJECT_VERSION})

# Link libraries
target_link_libraries(fcppGL PRIVATE fcpp)

# Copying shaders and fonts into build
copyGraphicsFolders(fcppGL $<TARGET_FILE_DIR:fcpp> $<TARGET_FILE_DIR:fcppGL>)

# Targets compile options
target_compile_options(fcpp PUBLIC "-DFCPP_CONFIGURATION=FCPP_CONFIGURATION_DEPENDENT")
#target_compile_options(fcpp PUBLIC "-DPROFILER")

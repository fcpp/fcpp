# CMake 3.9 or newer
cmake_minimum_required(VERSION 3.9)

# Defining functions
function(copyGraphicsFolders target source dest) # ${PROJECT_SOURCE_DIR}/lib/graphics $<TARGET_FILE_DIR:fcpp>
	add_custom_command(TARGET ${ARGV0} POST_BUILD
					   COMMAND ${CMAKE_COMMAND} -E copy_directory
							${ARGV1}/shaders/ ${ARGV2}/shaders/)
	add_custom_command(TARGET ${ARGV0} POST_BUILD
					   COMMAND ${CMAKE_COMMAND} -E copy_directory
							${ARGV1}/fonts/ ${ARGV2}/fonts/)
endfunction()
function(copyTextureFolder target source dest)
    add_custom_command(TARGET ${ARGV0} POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                            ${ARGV1}/textures/ ${ARGV2}/textures/)
endfunction()

# Project name, version and description
project(fcpp
	VERSION 0.6.6
	DESCRIPTION "FieldCalc++ is an efficient C++14 implementation of the Pure Field Calculus."
)

# Compiling options
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native")
option(FCPP_BUILD_GL "Build OpenGL features.")
if(FCPP_BUILD_GL)
add_compile_options("-DFCPP_GUI")
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
endif(FCPP_BUILD_GL)
if(WIN32)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
endif(WIN32)

# Source list
set(SOURCE_LIST lib/beautify.cpp
                lib/common.cpp
                lib/common/algorithm.cpp
                lib/common/multitype_map.cpp
                lib/common/mutex.cpp
                lib/common/ostream.cpp
                lib/common/plot.cpp
                lib/common/profiler.cpp
                lib/common/quaternion.cpp
                lib/common/random_access_map.cpp
                lib/common/serialize.cpp
                lib/common/tagged_tuple.cpp
                lib/common/traits.cpp
                lib/component.cpp
                lib/component/base.cpp
                lib/component/calculus.cpp
                lib/component/identifier.cpp
                lib/component/logger.cpp
                lib/component/randomizer.cpp
                lib/component/scheduler.cpp
                lib/component/storage.cpp
                lib/component/timer.cpp
                lib/coordination.cpp
                lib/coordination/collection.cpp
                lib/coordination/election.cpp
                lib/coordination/geometry.cpp
                lib/coordination/spreading.cpp
                lib/coordination/time.cpp
                lib/coordination/utils.cpp
                lib/data.cpp
                lib/data/color.cpp
                lib/data/field.cpp
                lib/data/shape.cpp
                lib/data/tuple.cpp
                lib/data/vec.cpp
                lib/deployment.cpp
                lib/deployment/hardware_connector.cpp
                lib/deployment/hardware_identifier.cpp
                lib/deployment/hardware_logger.cpp
                lib/deployment/os.cpp
                lib/fcpp.cpp
                lib/internal.cpp
                lib/internal/context.cpp
                lib/internal/flat_ptr.cpp
                lib/internal/trace.cpp
                lib/internal/twin.cpp
                lib/option.cpp
                lib/option/aggregator.cpp
                lib/option/connect.cpp
                lib/option/distribution.cpp
                lib/option/metric.cpp
                lib/option/sequence.cpp
                lib/settings.cpp
                lib/simulation.cpp
                lib/simulation/batch.cpp
                lib/simulation/simulated_connector.cpp
                lib/simulation/simulated_positioner.cpp
                lib/simulation/spawner.cpp
)
if(FCPP_BUILD_GL) # append source files with OpenGL features
list(APPEND SOURCE_LIST lib/graphics/camera.cpp
                        lib/graphics/renderer.cpp
                        lib/graphics/shader.cpp
                        lib/graphics/shapes.cpp
)
endif(FCPP_BUILD_GL)

# Library target
if(WIN32)
add_library(fcpp STATIC ${SOURCE_LIST})
else(WIN32)
add_library(fcpp SHARED ${SOURCE_LIST})
endif(WIN32)

# VERSION property
set_target_properties(fcpp PROPERTIES VERSION &{PROJECT_VERSION})

# Add external libraries
if(FCPP_BUILD_GL)
add_library(glad ./external/glad-core-3.3/include/KHR/khrplatform.h
                 ./external/glad-core-3.3/include/glad/glad.h
                 ./external/glad-core-3.3/src/glad.c)
add_library(stb_image ./external/stb_image/stb_image.h
                      ./external/stb_image/stb_image.cpp)
add_subdirectory(./external/glm-0.9.9.8/glm/)
add_subdirectory(./external/glfw-3.3.2)
add_subdirectory(./external/freetype-2.10.4)
endif(FCPP_BUILD_GL)

# Include directories
target_include_directories(fcpp PUBLIC ./)
if(FCPP_BUILD_GL)
target_include_directories(glad PUBLIC ./external/glad-core-3.3/include/)
target_include_directories(stb_image PUBLIC ./external/stb_image/)
target_include_directories(fcpp PUBLIC ./external/
                                       ./external/glad-core-3.3/include/
                                       ./external/glm-0.9.9.8/glm
                                       ./external/glfw-3.3.2/include/
                                       ./external/freetype-2.10.4/devel/)
endif(FCPP_BUILD_GL)

# Link libraries to main target fcpp
if(FCPP_BUILD_GL)
target_link_libraries(fcpp PUBLIC glad
                                  glm
                                  glfw
                                  stb_image
                                  freetype)
endif(FCPP_BUILD_GL)								  

# Copying shaders and fonts into build
if(FCPP_BUILD_GL)
copyGraphicsFolders(fcpp ${PROJECT_SOURCE_DIR}/lib/graphics $<TARGET_FILE_DIR:fcpp>)
endif(FCPP_BUILD_GL)	

# Targets compile options
if(UNIX AND FCPP_BUILD_GL)
target_compile_options(stb_image PUBLIC "-fPIC")
target_compile_options(glad PUBLIC "-fPIC")
target_compile_options(freetype PUBLIC "-fPIC")
endif(UNIX AND FCPP_BUILD_GL)

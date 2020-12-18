# CMake 3.9 or newer
cmake_minimum_required(VERSION 3.9)

# C++14 or higher
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# project name, version and description
project(fcpp
    VERSION 0.6.22
    DESCRIPTION "FieldCalc++ is an efficient C++14 implementation of the Pure Field Calculus."
)

# library target
add_library(fcpp SHARED
    lib/beautify.cpp
    lib/common.cpp
    lib/common/algorithm.cpp
    lib/common/multitype_map.cpp
    lib/common/mutex.cpp
    lib/common/ostream.cpp
    lib/common/plot.cpp
    lib/common/profiler.cpp
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
    lib/data/tuple.cpp
    lib/data/vec.cpp
    lib/deployment.cpp
    lib/deployment/hardware_connector.cpp
    lib/deployment/hardware_identifier.cpp
    lib/deployment/hardware_logger.cpp
    lib/deployment/os.cpp
    lib/fcpp.cpp
    lib/graphics/camera.cpp
    lib/graphics/renderer.cpp
    lib/graphics/shader.cpp
    lib/graphics/shapes.cpp
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
    lib/simulation/displayer.cpp
    lib/simulation/simulated_connector.cpp
    lib/simulation/simulated_positioner.cpp
    lib/simulation/spawner.cpp
)

# VERSION property
set_target_properties(fcpp PROPERTIES VERSION &{PROJECT_VERSION})

# public API declaration
set_target_properties(fcpp PROPERTIES PUBLIC_HEADER include/fcpp.h)

# adding external libraries
add_library(glad ./external/glad-core-3.3/include/KHR/khrplatform.h
                 ./external/glad-core-3.3/include/glad/glad.h
                 ./external/glad-core-3.3/src/glad.c)
add_library(stb_image ./external/stb_image/stb_image.h
                      ./external/stb_image/stb_image.cpp)
add_subdirectory(./external/glm-0.9.9.8/glm/)
add_subdirectory(./external/glfw-3.3.2/)
add_subdirectory(./external/freetype-2.10.4/)

# include directories
target_include_directories(glad      PRIVATE ./external/glad-core-3.3/include/)
target_include_directories(stb_image PRIVATE ./external/stb_image/)
target_include_directories(fcpp      PRIVATE ./
                                             ./external/
                                             ./external/glad-core-3.3/include/
                                             ./external/glm-0.9.9.8/glm/
                                             ./external/glfw-3.3.2/include/
                                             ./external/freetype-2.10.4/devel/)

# link libraries
target_link_libraries(fcpp PRIVATE glad
                                   glm
                                   glfw
                                   stb_image
                                   freetype)

# MinGW support to C++ threads
if(MINGW)
    add_subdirectory(./external/mingw-std-threads-master)
    target_include_directories(fcpp PRIVATE ./external/mingw-std-threads-master/)
    target_link_libraries(fcpp PRIVATE mingw_stdthreads)
    option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
endif(MINGW)

#target compile options
if(UNIX)
    target_compile_options(glad PUBLIC "-fPIC")
    target_compile_options(freetype PUBLIC "-fPIC")
endif(UNIX)

# defining install rules
include(GNUInstallDirs)
install(TARGETS fcpp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
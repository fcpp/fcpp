cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

####################
# HELPER FUNCTIONS #
####################

function(fcpp_copy target source dest folder)
    if(EXISTS ${source}/${folder})
        add_custom_command(
            TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${source}/${folder} ${dest}/${folder}
        )
    endif()
endfunction()

macro(fcpp_setup)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    if(NOT CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        # -march=native is not supported by arm64
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
    endif()
    if(FCPP_DISABLE_THREADS)
        set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -DFCPP_DISABLE_THREADS")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DFCPP_DISABLE_THREADS")
    else(FCPP_DISABLE_THREADS)
        set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -pthread")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread")
    endif(FCPP_DISABLE_THREADS)
    if(WIN32)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Os")
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set(BUILD_SHARED_LIBS OFF)
        set(CMAKE_EXE_LINKER_FLAGS "-static")
        if(FCPP_BUILD_GL)
            set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec True)
        endif(FCPP_BUILD_GL)
    endif(WIN32)
	if(FCPP_BUILD_MPI)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFCPP_MPI")
    endif(FCPP_BUILD_MPI)
    if(FCPP_BUILD_TESTS)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test)
    endif(FCPP_BUILD_TESTS)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/run)
endmacro()

function(fcpp_target target_executable enable_gui)
    get_filename_component(target_name ${target_executable} NAME_WE)
    add_executable(${target_name} ${target_executable})
    target_link_libraries(${target_name} PRIVATE fcpp)
    target_include_directories(${target_name} PRIVATE .)
    if(${enable_gui} STREQUAL ON)
        if(FCPP_BUILD_GL)
            fcpp_copy(${target_name} ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} textures)
            target_compile_options(${target_name} PUBLIC -DFCPP_CONFIGURATION=FCPP_CONFIGURATION_DEPENDENT -DFCPP_GUI)
        endif()
    endif()
    fcpp_copy(${target_name} ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} input)
    set_target_properties(${target_name} PROPERTIES PREFIX "run/")
endfunction()

function(fcpp_header target_executable enable_gui)
    get_filename_component(target_name ${target_executable} NAME_WE)
    add_library(${target_name} STATIC ${target_executable})
    target_link_libraries(${target_name} PRIVATE fcpp)
    target_include_directories(${target_name} PRIVATE .)
    set_target_properties(${target_name} PROPERTIES PREFIX "run/")
    if(${enable_gui} STREQUAL ON)
        if(FCPP_BUILD_GL)
            fcpp_copy(${target_name} ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} textures)
            target_compile_options(${target_name} PUBLIC -DFCPP_CONFIGURATION=FCPP_CONFIGURATION_DEPENDENT -DFCPP_GUI)
        endif()
    endif()
    fcpp_copy(${target_name} ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} input)
endfunction()

function(fcpp_headers target_name enable_gui source_list extra_libs)
    add_library(${target_name} SHARED ${source_list})
    target_link_libraries(${target_name} PRIVATE fcpp ${extra_libs})
    target_include_directories(${target_name} PRIVATE .)
    set_target_properties(${target_name} PROPERTIES PREFIX "run/")
    if(${enable_gui} STREQUAL ON)
        if(FCPP_BUILD_GL)
            fcpp_copy(${target_name} ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} textures)
            target_compile_options(${target_name} PUBLIC -DFCPP_CONFIGURATION=FCPP_CONFIGURATION_DEPENDENT -DFCPP_GUI)
        endif()
    endif()
    fcpp_copy(${target_name} ${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} input)
endfunction()

function(fcpp_test target_executable)
    get_filename_component(target_name ${target_executable} NAME_WE)
    if(FCPP_BUILD_TESTS)
        add_executable(${target_name}_test ${target_executable})
        target_link_libraries(${target_name}_test PRIVATE
            fcpp
            fcpp_testlib
            gtest
            gtest_main
        )
        target_include_directories(${target_name}_test PRIVATE .)
        set_target_properties(${target_name}_test PROPERTIES PREFIX "test/")
        add_test(NAME ${target_name}_test COMMAND ${target_name}_test)
    endif(FCPP_BUILD_TESTS)
endfunction()

######################
# MAIN PROJECT SETUP #
######################

project(
    fcpp
    VERSION 0.8.1
    DESCRIPTION "FieldCalc++ is an efficient C++14 implementation of the Pure Field Calculus."
)
option(FCPP_BUILD_GL        "Build OpenGL features."          OFF)
option(FCPP_BUILD_MPI       "Build MPI features."             OFF)
option(FCPP_BUILD_TESTS     "Build Google Tests features."    OFF)
option(FCPP_INTERNAL_TESTS  "Build internal tests for FCPP."  ON)
fcpp_setup()
set(
    SOURCE_LIST
    lib/beautify.cpp
    lib/cloud.cpp
    lib/cloud/graph_connector.cpp
    lib/cloud/graph_spawner.cpp
    lib/common.cpp
    lib/common/algorithm.cpp
    lib/common/immutable_map.cpp
    lib/common/multitype_map.cpp
    lib/common/mutex.cpp
    lib/common/number_sequence.cpp
    lib/common/ostream.cpp
    lib/common/plot.cpp
    lib/common/profiler.cpp
    lib/common/quaternion.cpp
    lib/common/random_access_map.cpp
    lib/common/serialize.cpp
    lib/common/tagged_tuple.cpp
    lib/common/traits.cpp
    lib/common/type_sequence.cpp
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
    lib/coordination/basics.cpp
    lib/coordination/collection.cpp
    lib/coordination/election.cpp
    lib/coordination/geometry.cpp
    lib/coordination/spreading.cpp
    lib/coordination/time.cpp
    lib/coordination/utils.cpp
    lib/data.cpp
    lib/data/bloom.cpp
    lib/data/color.cpp
    lib/data/field.cpp
    lib/data/hyperloglog.cpp
    lib/data/ordered.cpp
    lib/data/shape.cpp
    lib/data/tuple.cpp
    lib/data/vec.cpp
    lib/deployment.cpp
    lib/deployment/hardware_connector.cpp
    lib/deployment/hardware_identifier.cpp
    lib/deployment/hardware_logger.cpp
    lib/deployment/os.cpp
    lib/deployment/persister.cpp
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
    lib/option/filter.cpp
    lib/option/functor.cpp
    lib/option/metric.cpp
    lib/option/sequence.cpp
    lib/settings.cpp
    lib/simulation.cpp
    lib/simulation/batch.cpp
    lib/simulation/simulated_connector.cpp
    lib/simulation/simulated_map.cpp
    lib/simulation/simulated_positioner.cpp
    lib/simulation/spawner.cpp
)
if(FCPP_BUILD_GL)
    list(
        APPEND SOURCE_LIST
        lib/graphics/camera.cpp
        lib/graphics/renderer.cpp
        lib/graphics/shader.cpp
        lib/graphics/shapes.cpp
        lib/simulation/displayer.cpp
    )
endif(FCPP_BUILD_GL)
if(WIN32)
    add_library(fcpp STATIC ${SOURCE_LIST})
else(WIN32)
    add_library(fcpp SHARED ${SOURCE_LIST})
endif(WIN32)
target_include_directories(fcpp PUBLIC .)
if(FCPP_BUILD_GL)
    fcpp_copy(fcpp ${PROJECT_SOURCE_DIR}/lib/graphics ${CMAKE_BINARY_DIR} shaders)
    fcpp_copy(fcpp ${PROJECT_SOURCE_DIR}/lib/graphics ${CMAKE_BINARY_DIR} fonts)
endif(FCPP_BUILD_GL)

######################
# EXTERNAL LIBRARIES #
######################

add_library(
        stb_image
        ./external/stb_image/stb_image.h
        ./external/stb_image/stb_image.cpp
)
target_include_directories(stb_image PUBLIC ./external/stb_image)
if(FCPP_BUILD_MPI)
    find_package(MPI REQUIRED)
    target_link_libraries(
        fcpp PUBLIC
        stb_image
        ${MPI_LIBRARIES}
    )
    target_include_directories(fcpp PUBLIC ${MPI_INCLUDE_PATH})
    if(MPI_COMPILE_FLAGS)
        set_target_properties(fcpp PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
    endif()
    if(MPI_LINK_FLAGS)
        set_target_properties(fcpp PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
    endif()
else()
    target_link_libraries(
        fcpp PUBLIC
        stb_image
    )
endif(FCPP_BUILD_MPI)
set_target_properties(${target_name} PROPERTIES PREFIX "run/")
if(UNIX)
    target_compile_options(stb_image PUBLIC -fPIC)
endif(UNIX)
if(FCPP_BUILD_GL)
    option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
    option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
    option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
    add_library(
        glad
        ./external/glad-core-3.3/include/KHR/khrplatform.h
        ./external/glad-core-3.3/include/glad/glad.h
        ./external/glad-core-3.3/src/glad.c
    )
    add_subdirectory(./external/glm-0.9.9.8/glm)
    add_subdirectory(./external/glfw-3.3.4)
    add_subdirectory(./external/freetype-2.11.0)
    target_include_directories(glad PUBLIC ./external/glad-core-3.3/include)
    target_include_directories(
        fcpp PUBLIC
        ./external
        ./external/glad-core-3.3/include
        ./external/glm-0.9.9.8/glm
        ./external/glfw-3.3.4/include
        ./external/freetype-2.11.0/devel
    )
    target_link_libraries(
        fcpp PUBLIC
        glad
        glm
        glfw
        freetype
    )
    if(UNIX)
        target_compile_options(glad PUBLIC -fPIC)
        target_compile_options(freetype PUBLIC -fPIC)
    endif(UNIX)
endif(FCPP_BUILD_GL)

################
# GOOGLE TESTS #
################

if(FCPP_BUILD_TESTS)
    cmake_minimum_required(VERSION 3.14)
    include(FetchContent)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/f5e592d8ee5ffb1d9af5be7f715ce3576b8bf9c4.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(CTest)
    set(
        TEST_HELPERS
        test/fake_os.cpp
        test/helper.cpp
        test/test_net.cpp
    )
    add_library(fcpp_testlib STATIC ${TEST_HELPERS})
    target_link_libraries(fcpp_testlib PRIVATE
        fcpp
        gtest
    )
    if(FCPP_INTERNAL_TESTS)
        fcpp_test(test/cloud/graph_connector.cpp)
        fcpp_test(test/cloud/graph_spawner.cpp)
        fcpp_test(test/common/algorithm.cpp)
        fcpp_test(test/common/immutable_map.cpp)
        fcpp_test(test/common/multitype_map.cpp)
        fcpp_test(test/common/mutex.cpp)
        fcpp_test(test/common/number_sequence.cpp)
        fcpp_test(test/common/option.cpp)
        fcpp_test(test/common/ostream.cpp)
        fcpp_test(test/common/plot.cpp)
        fcpp_test(test/common/profiler.cpp)
        fcpp_test(test/common/quaternion.cpp)
        fcpp_test(test/common/random_access_map.cpp)
        fcpp_test(test/common/serialize.cpp)
        fcpp_test(test/common/tagged_tuple.cpp)
        fcpp_test(test/common/traits.cpp)
        fcpp_test(test/common/type_sequence.cpp)
        fcpp_test(test/component/base.cpp)
        fcpp_test(test/component/calculus.cpp)
        fcpp_test(test/component/identifier.cpp)
        fcpp_test(test/component/logger.cpp)
        fcpp_test(test/component/randomizer.cpp)
        fcpp_test(test/component/scheduler.cpp)
        fcpp_test(test/component/storage.cpp)
        fcpp_test(test/component/timer.cpp)
        fcpp_test(test/coordination/basics.cpp)
        fcpp_test(test/coordination/collection.cpp)
        fcpp_test(test/coordination/election.cpp)
        fcpp_test(test/coordination/geometry.cpp)
        fcpp_test(test/coordination/spreading.cpp)
        fcpp_test(test/coordination/time.cpp)
        fcpp_test(test/coordination/utils.cpp)
        fcpp_test(test/data/bloom.cpp)
        fcpp_test(test/data/color.cpp)
        fcpp_test(test/data/field.cpp)
        fcpp_test(test/data/hyperloglog.cpp)
        fcpp_test(test/data/ordered.cpp)
        fcpp_test(test/data/tuple.cpp)
        fcpp_test(test/data/vec.cpp)
        fcpp_test(test/deployment/hardware_connector.cpp)
        fcpp_test(test/deployment/hardware_identifier.cpp)
        fcpp_test(test/deployment/hardware_logger.cpp)
        fcpp_test(test/deployment/persister.cpp)
        fcpp_test(test/general/collection_compare.cpp)
        fcpp_test(test/general/embedded.cpp)
        fcpp_test(test/general/slow_distance.cpp)
        fcpp_test(test/internal/context.cpp)
        fcpp_test(test/internal/flat_ptr.cpp)
        fcpp_test(test/internal/trace.cpp)
        fcpp_test(test/internal/twin.cpp)
        fcpp_test(test/option/aggregator.cpp)
        fcpp_test(test/option/connect.cpp)
        fcpp_test(test/option/distribution.cpp)
        fcpp_test(test/option/filter.cpp)
        fcpp_test(test/option/functor.cpp)
        fcpp_test(test/option/metric.cpp)
        fcpp_test(test/option/sequence.cpp)
        fcpp_test(test/simulation/batch.cpp)
        fcpp_test(test/simulation/simulated_connector.cpp)
        fcpp_test(test/simulation/simulated_positioner.cpp)
        fcpp_test(test/simulation/simulated_map.cpp)
        fcpp_test(test/simulation/spawner.cpp)
    endif(FCPP_INTERNAL_TESTS)
endif(FCPP_BUILD_TESTS)

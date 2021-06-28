cmake_minimum_required(VERSION 3.9)

####################
# HELPER FUNCTIONS #
####################

function(fcpp_gui_copy target source dest folder)
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${source}/${folder} ${dest}/${folder}
    )
endfunction()

macro(fcpp_setup)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -march=native")
    if(WIN32)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
        set(BUILD_SHARED_LIBS OFF)
        set(CMAKE_EXE_LINKER_FLAGS "-static")
    endif(WIN32)
endmacro()

function(fcpp_target target_name target_executable enable_gui)
    add_executable(${target_name} ${target_executable})
    target_link_libraries(${target_name} PRIVATE fcpp)
    target_include_directories(${target_name} PRIVATE .)
    if(${enable_gui} STREQUAL ON)
        fcpp_gui_copy(${target_name} ${PROJECT_SOURCE_DIR} $<TARGET_FILE_DIR:${target_name}> textures)
        target_compile_options(${target_name} PUBLIC -DFCPP_CONFIGURATION=FCPP_CONFIGURATION_DEPENDENT -DFCPP_GUI)
    endif()
endfunction()

######################
# MAIN PROJECT SETUP #
######################

project(
    fcpp
    VERSION 0.8.1
    DESCRIPTION "FieldCalc++ is an efficient C++14 implementation of the Pure Field Calculus."
)
option(FCPP_BUILD_GL "Build OpenGL features." OFF)
option(FCPP_BUILD_TESTS "Build Google Tests." OFF)
fcpp_setup()
set(
    SOURCE_LIST
    lib/beautify.cpp
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
    fcpp_gui_copy(fcpp ${PROJECT_SOURCE_DIR}/lib/graphics ${CMAKE_BINARY_DIR} shaders)
    fcpp_gui_copy(fcpp ${PROJECT_SOURCE_DIR}/lib/graphics ${CMAKE_BINARY_DIR} fonts)
endif(FCPP_BUILD_GL)

######################
# EXTERNAL LIBRARIES #
######################

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
    add_library(
        stb_image
        ./external/stb_image/stb_image.h
        ./external/stb_image/stb_image.cpp
    )
    add_subdirectory(./external/glm-0.9.9.8/glm)
    add_subdirectory(./external/glfw-3.3.2)
    add_subdirectory(./external/freetype-2.10.4)
    target_include_directories(glad PUBLIC ./external/glad-core-3.3/include)
    target_include_directories(stb_image PUBLIC ./external/stb_image)
    target_include_directories(
        fcpp PUBLIC
        ./external
        ./external/glad-core-3.3/include
        ./external/glm-0.9.9.8/glm
        ./external/glfw-3.3.2/include
        ./external/freetype-2.10.4/devel
    )
    target_link_libraries(
        fcpp PUBLIC
        glad
        glm
        glfw
        stb_image
        freetype
    )
    if(UNIX)
        target_compile_options(stb_image PUBLIC -fPIC)
        target_compile_options(glad PUBLIC -fPIC)
        target_compile_options(freetype PUBLIC -fPIC)
    endif(UNIX)
endif(FCPP_BUILD_GL)

################
# GOOGLE TESTS #
################

if(FCPP_BUILD_TESTS)
    find_package(GTest)
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
        GTest::GTest
    )
    set(
        TEST_FILES
        test/common/algorithm.cpp
        test/common/multitype_map.cpp
        test/common/mutex.cpp
        test/common/option.cpp
        test/common/ostream.cpp
        test/common/plot.cpp
        test/common/profiler.cpp
        test/common/quaternion.cpp
        test/common/random_access_map.cpp
        test/common/serialize.cpp
        test/common/tagged_tuple.cpp
        test/common/traits.cpp
        test/component/base.cpp
        test/component/calculus.cpp
        test/component/identifier.cpp
        test/component/logger.cpp
        test/component/randomizer.cpp
        test/component/scheduler.cpp
        test/component/storage.cpp
        test/component/timer.cpp
        test/coordination/collection.cpp
        test/coordination/election.cpp
        test/coordination/geometry.cpp
        test/coordination/spreading.cpp
        test/coordination/time.cpp
        test/coordination/utils.cpp
        test/data/color.cpp
        test/data/field.cpp
        test/data/tuple.cpp
        test/data/vec.cpp
        test/deployment/hardware_connector.cpp
        test/deployment/hardware_identifier.cpp
        test/deployment/hardware_logger.cpp
        test/general/collection_compare.cpp
        test/general/embedded.cpp
        test/general/slow_distance.cpp
        test/internal/context.cpp
        test/internal/flat_ptr.cpp
        test/internal/trace.cpp
        test/internal/twin.cpp
        test/option/aggregator.cpp
        test/option/connect.cpp
        test/option/distribution.cpp
        test/option/metric.cpp
        test/option/sequence.cpp
        test/simulation/batch.cpp
        test/simulation/simulated_connector.cpp
        test/simulation/simulated_positioner.cpp
        test/simulation/spawner.cpp
    )
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)
    foreach(TESTFILE IN LISTS TEST_FILES)
        get_filename_component(TESTNAME ${TESTFILE} NAME_WE)
        add_executable(${TESTNAME} ${TESTFILE})
        target_link_libraries(${TESTNAME} PRIVATE
            fcpp
            fcpp_testlib
            GTest::GTest
            GTest::Main
        )
        set_target_properties(${TESTNAME} PROPERTIES PREFIX "tests/")
        if(${CMAKE_VERSION} VERSION_LESS "3.10.3")
            gtest_discover_tests(${TESTNAME} TIMEOUT 240)
        else()
            gtest_discover_tests(${TESTNAME} DISCOVERY_TIMEOUT 240)
        endif()
    endforeach()
endif(FCPP_BUILD_TESTS)

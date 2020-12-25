# CMake 3.9 or newer
cmake_minimum_required(VERSION 3.9)

# project name, version and description
project(fcppGL
	VERSION 0.1.0
	DESCRIPTION "FieldCalc++ OpenGL simulator."
)

# C++14 or higher
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# adding fcpp_master
if(MINGW)
    option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
endif(MINGW)
add_subdirectory(./src/)

# executable target
add_executable(fcppGL ./src/test/simulation/displayer.cpp)

# VERSION property
set_target_properties(fcppGL PROPERTIES VERSION &{PROJECT_VERSION})

# include directories
if(MINGW)
target_include_directories(fcppGL PRIVATE ./src/external/mingw-std-threads-master)
endif(MINGW)
target_include_directories(fcppGL PRIVATE ./src/
                                          ./src/external/
                                          ./src/external/glad-core-3.3/include/
                                          ./src/external/glm-0.9.9.8/glm
                                          ./src/external/glfw-3.3.2/include/
                                          ./src/external/freetype-2.10.4/devel/)


# link libraries
if(MINGW)
target_link_libraries(fcppGL PRIVATE mingw_stdthreads)
endif(MINGW)
target_link_libraries(fcppGL PRIVATE fcpp_master
                                     glm
									 glfw
									 stb_image
									 freetype)


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
#target_compile_options(fcppGL PUBLIC "-DFCPP_REFRESH_RATE=(1.0/144)")

# defining install rules
include(GNUInstallDirs)
install(TARGETS fcppGL
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)











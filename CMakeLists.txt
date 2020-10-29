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

# adding libraries to build
add_library(glad ./src/external/glad-core-3.3/include/KHR/khrplatform.h
                 ./src/external/glad-core-3.3/include/glad/glad.h
                 ./src/external/glad-core-3.3/src/glad.c)
add_library(stb_image ./src/external/stb_image/stb_image.h
                      ./src/external/stb_image/stb_image.cpp)
option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
add_subdirectory(./src/)
add_subdirectory(./src/external/glm-0.9.9.8/glm/)
add_subdirectory(./src/external/glfw-3.3.2)

# executable target
add_executable(fcppGL ./src/test/simulation/displayer.cpp)

# VERSION property
set_target_properties(fcppGL PROPERTIES VERSION &{PROJECT_VERSION})

# include directories
target_include_directories(glad PRIVATE ./src/external/glad-core-3.3/include/)
target_include_directories(stb_image PRIVATE ./src/external/stb_image/)
target_include_directories(fcppGL PRIVATE ./src/
                                          ./src/external/
                                          ./src/external/mingw-std-threads-master
										  ./src/external/glad-core-3.3/include/
										  ./src/external/glm-0.9.9.8/glm
										  ./src/external/glfw-3.3.2/include/)

# link libraries
target_link_libraries(fcppGL PRIVATE mingw_stdthreads
                                     fcpp_master
                                     glad
                                     glm
									 glfw
									 stb_image)

# copying shaders and textures into build
#add_custom_command(TARGET fcppGL PRE_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_directory
#                       ${CMAKE_SOURCE_DIR}/src/textures/ $<TARGET_FILE_DIR:fcppGL>/textures/)
					   
#add_custom_command(TARGET fcppGL PRE_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_directory
#                       ${CMAKE_SOURCE_DIR}/src/shaders/ $<TARGET_FILE_DIR:fcppGL>/shaders/)

# defining install rules
include(GNUInstallDirs)
install(TARGETS fcppGL
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)










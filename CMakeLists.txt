cmake_minimum_required(VERSION 3.15)
project(JSL)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Force the compiler to use C++20 for all targets
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(WIN32)
    add_compile_definitions(NOMINMAX)
endif()

file(GLOB_RECURSE JSL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
#file(GLOB_RECURSE JSL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_library(JSL STATIC
${JSL_SOURCES}
)


#give us a nice name
set_target_properties(JSL PROPERTIES PREFIX "")
set_target_properties(JSL PROPERTIES SUFFIX "")
set_target_properties(JSL PROPERTIES OUTPUT_NAME "JSL.lib")


target_include_directories(JSL
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src
)

# If your library needs specific standard features (like C++20)
target_compile_features(JSL PUBLIC cxx_std_20)

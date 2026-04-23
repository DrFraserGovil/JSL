cmake_minimum_required(VERSION 3.15)
project(jsl)


file(GLOB_RECURSE JSL_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_library(jsl STATIC
${JSL_SOURCES}
)


#give us a nice name
set_target_properties(jsl PROPERTIES PREFIX "")
set_target_properties(jsl PROPERTIES OUTPUT_NAME "jack_standard_library")


target_include_directories(jsl 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE src
)

# If your library needs specific standard features (like C++20)
target_compile_features(jsl PUBLIC cxx_std_20)
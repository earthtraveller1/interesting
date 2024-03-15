cmake_minimum_required(VERSION 3.5)

project(interesting LANGUAGES C)

option(INTERESTING_BUILD_TESTS "Build tests" ON)

aux_source_directory(src SOURCES)
add_executable(${PROJECT_NAME} ${SOURCES})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

configure_file(cmake/config.ini config.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
endif()

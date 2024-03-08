cmake_minimum_required(VERSION 3.5)

project(interesting)

aux_source_directory(src SOURCES)
add_executable(${PROJECT_NAME} ${SOURCES})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

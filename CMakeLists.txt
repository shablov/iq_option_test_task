cmake_minimum_required(VERSION 3.9)

set(CMAKE_CXX_STANDARD 17)

project(iq_option_test_task)

add_custom_target(task SOURCES task.txt)

add_subdirectory(core)
add_subdirectory(generator)
add_subdirectory(libs)
add_subdirectory(statistics_service)

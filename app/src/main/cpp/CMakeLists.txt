cmake_minimum_required(VERSION 3.22.1)

project("letsdraw")

add_library(
        drawengine
        SHARED
        native-lib.cpp
        drawengine.cpp
)

find_library(
        log-lib
        log
)

target_link_libraries(
        drawengine
        ${log-lib}
)
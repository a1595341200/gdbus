cmake_minimum_required(VERSION 3.10)

set(PROJECT_N framework)
project(${PROJECT_N} VERSION 1.0)
find_package(PkgConfig)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)

# a macro that gets all of the header containing directories.
MACRO(header_directories return_list includes_base_folder extention)
    FILE(GLOB_RECURSE new_list ${includes_base_folder}/*.${extention})
    SET(dir_list "")

    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()

    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

# using said macro.
header_directories(INCLUDES ${PROJECT_SOURCE_DIR}/include/ hpp)

message("src files:")

foreach(file ${SRCS})
    message(STATUS ${file})
endforeach()

message("include directories:")

foreach(dir ${INCLUDES})
    message(STATUS ${dir})
endforeach()

add_library(${PROJECT_N} ${SRCS})
set(GLIB_DIR
    "/usr/include/glib-2.0"
    "/usr/include/gio-unix-2.0"
    "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
)
target_include_directories(${PROJECT_N} PUBLIC include ${GLIB_DIR})
target_link_libraries(${PROJECT_N} gio-2.0 glib-2.0 pthread gobject-2.0 gmodule-2.0 gthread-2.0 rt)
set(LIBRARY_OUTPUT_DIRECTORY lib)
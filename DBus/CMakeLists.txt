cmake_minimum_required(VERSION 3.10...3.28)
project(IPCDBus)

set(CMAKE_BUILD_TYPE Debug)

# Find the dbus-1 package
find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS1 REQUIRED dbus-1)

function(compile_executables target_name file_path)
    add_executable(${target_name} ${file_path})

    target_link_libraries(${target_name}
        PRIVATE
            pthread
            ${DBUS1_LIBRARIES}
    )

    target_include_directories(${target_name}
        PRIVATE
            ${DBUS1_INCLUDE_DIRS}
    )

    target_compile_options(${target_name}
        PRIVATE
            ${DBUS1_CFLAGS_OTHER}
    )
endfunction()

file(GLOB_RECURSE C_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")
file(GLOB_RECURSE CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

foreach(file_path ${C_FILES})
    get_filename_component(target_name ${file_path} NAME_WE)
    compile_executables(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/${file_path})
endforeach()

foreach(file_path ${CPP_FILES})
    get_filename_component(target_name ${file_path} NAME_WE)
    compile_executables(${target_name} ${CMAKE_CURRENT_SOURCE_DIR}/${file_path})
endforeach()
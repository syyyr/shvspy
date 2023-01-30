cmake_minimum_required(VERSION 3.18.4)
project(shvspy LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_WIN32_EXECUTABLE ON) # Don't open a console window for Windows apps on Release mode.
endif()

if(EMSCRIPTEN)
    add_link_options("-sASYNCIFY")
    add_link_options("-Os")
endif()

add_subdirectory(3rdparty/necrolog)
add_subdirectory(3rdparty/libshv)

if(USE_QT6)
    if(EMSCRIPTEN)
        find_package(Qt6 COMPONENTS Core Widgets Gui Network Svg)
    else()
        find_package(Qt6 COMPONENTS Core Widgets Gui Network Svg SerialPort WebSockets)
    endif()
    set(Qt_FOUND ${Qt6_FOUND})
else()
    find_package(Qt5 QUIET COMPONENTS Core Widgets Gui Network Svg SerialPort)
    set(Qt_FOUND ${Qt5_FOUND})

    # Qt5 doesn't have add qt_add_executable so this is a polyfill for that, since we already support Qt6.
    function(qt_add_executable)
        add_executable(${ARGV})
    endfunction()
endif()


if(Qt_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    add_subdirectory(shvspy)
else()
    message(FATAL_ERROR "Qt${QT_VERSION_MAJOR} not found")
endif()

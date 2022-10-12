cmake_minimum_required(VERSION 3.18.4)
project(shvspy LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# include(CTest)
set(CMAKE_SHARED_LIBRARY_PREFIX "") # we don't want CMake to prepend "lib" to our libraries, we prefer adding that ourselves

if(DEFINED ENV{RUN_LUPDATE})
  message(WARNING "RUN_LUPDATE env variable is set! Translation files will be refreshed.")
endif()

set(PROJECT_TRANSLATIONS
  # cs_CZ
 # fi_FI
 # nl_NL
 # nl_BE
 # pl_PL
 # hu_HU
  )

#function(set_translations component langs)
#  foreach(lang ${langs})
#    set(TS_FILES ${TS_FILES} "${component}.${lang}.ts")
#  endforeach()

#  if(DEFINED ENV{RUN_LUPDATE})
#    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
#  else()
#    qt5_add_translation(QM_FILES ${TS_FILES})
#  endif()

#  target_sources(${component} PRIVATE ${QM_FILES})
#endfunction()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic ${CMAKE_CXX_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_WIN32_EXECUTABLE ON) # Don't open a console window for Windows apps on Release mode.
endif()

add_subdirectory(3rdparty/necrolog)
add_subdirectory(3rdparty/libshv)

function(target_enable_coroutines target_name)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${target_name} PUBLIC "-fcoroutines-ts")
    else()
        target_compile_options(${target_name} PUBLIC "-fcoroutines")
    endif()
endfunction()

find_package(Qt5 QUIET COMPONENTS Core Widgets Gui Network Svg SerialPort )
# find_package(QCoro5 REQUIRED COMPONENTS Core)

if(Qt5_FOUND)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    add_subdirectory(shvspy)
else()
    message(FATAL_ERROR "Qt5 not found")
endif()

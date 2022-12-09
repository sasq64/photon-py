cmake_minimum_required(VERSION 3.12)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
project(photon-py VERSION 1.0)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(ExternalProject)
include(FetchContent)

#FetchContent_Declare(freetype
#URL https://download.savannah.gnu.org/releases/freetype/freetype-2.12.1.tar.gz)
#FetchContent_Populate(glfw)
#add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)


set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

cmake_policy(SET CMP0072 NEW)

# System wide directories and flags

if (WIN32)
    add_compile_options(/utf-8)
    add_definitions(/D_CRT_SECURE_NO_WARNINGS /D_USE_MATH_DEFINES
        /D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
endif ()

if (APPLE)
    include_directories(SYSTEM /usr/local/include /opt/homebrew/include)
    link_directories(/usr/local/lib /opt/homebrew/lib)
    #add_compile_options(-target x86_64-apple-macos10.15)
endif ()

# Warning level setup #######################################################

# GCC/Clang common warnings
set(GCC_CLANG_WARNINGS
    -Werror
    -Wall
    -Wextra
    -Wnon-virtual-dtor
    -pedantic
    -Wno-psabi
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-unused-function)

set(MSVC_WARNINGS /W2 /wd4244)

# GCC specific warnings
set(GCC_WARNINGS -Wno-duplicated-branches -Wduplicated-cond -Wlogical-op
    -Wno-error=stringop-overflow
    -Wcast-align -Wno-missing-field-initializers)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(WARNINGS ${WARNINGS} ${GCC_WARNINGS} ${GCC_CLANG_WARNINGS})
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(WARNINGS ${WARNINGS} ${CLANG_WARNINGS} ${GCC_CLANG_WARNINGS})
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(WARNINGS ${WARNINGS} ${MSVC_WARNINGS})
endif ()

add_library(Warnings INTERFACE)
target_compile_options(Warnings INTERFACE ${WARNINGS})

add_library(Extra INTERFACE)

find_package(Threads REQUIRED)

add_subdirectory(external/pybind11)

add_library(Photon-cpp INTERFACE)
target_link_directories(Photon-cpp INTERFACE
    external/photon/3rdparty/lib/apple
    external/photon/Photon-cpp/lib
    external/photon/Common-cpp/lib
    external/photon/LoadBalancing-cpp/lib)
target_include_directories(Photon-cpp INTERFACE external/photon)

if (APPLE)

target_link_libraries(Photon-cpp INTERFACE
    "-framework Foundation" "-framework CFNetwork" "-framework Security" "-framework CoreFoundation"
)
target_compile_definitions(Photon-cpp INTERFACE _EG_IMAC_PLATFORM)

target_link_libraries(Photon-cpp INTERFACE
    crypto_debug_macosx
    Photon-cpp_debug_macosx
    Common-cpp_debug_macosx
    LoadBalancing-cpp_debug_macosx)
else()
    target_compile_definitions(Photon-cpp INTERFACE _EG_LINUX_PLATFORM)
    target_link_libraries(Photon-cpp INTERFACE
        LoadBalancingDebug64
        PhotonDebug64
        CommonDebug64)
endif()

pybind11_add_module(_photon src/photon.cpp)
target_link_libraries(_photon PUBLIC Extra Photon-cpp)
install(TARGETS _photon DESTINATION .)

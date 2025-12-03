cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(TestCoCa)

set(CMAKE_C_COMPILER "/usr/bin/clang-18")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-18")

macro(append_compiler_flags FLAGS)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${FLAGS}")
endmacro()

macro(append_linker_flags FLAGS)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()

macro(append_build_flags FLAGS)
    append_compiler_flags("${FLAGS}")
    append_linker_flags("${FLAGS}")
endmacro()

if(NOT CMAKE_BUILD_TYPE)
    message("Build type not set => setting 'Release' as default.")
    set(CMAKE_BUILD_TYPE "Release"  CACHE STRING "Release" FORCE)
endif()

message("Build type = " ${CMAKE_BUILD_TYPE})

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    add_definitions(-DRELEASE)
else()
    message(SEND_ERROR "Unknown build type. Use either Debug, Release. E.g.: -DCMAKE_BUILD_TYPE=Release")
endif()

if(NOT DEFINED BUILD_LIBS_32_BIT)
    set(BUILD_LIBS_32_BIT "No" CACHE STRING "Build only 32-bit libraries? (Yes/No)" FORCE)
endif()

message("Build only 32-bit libraries: " ${BUILD_LIBS_32_BIT})

if(BUILD_LIBS_32_BIT STREQUAL "Yes")
    set(BUILD_ALSO_TOOLS "No")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    message("Compiler = GCC")
    if(BUILD_LIBS_32_BIT STREQUAL "Yes")
        append_build_flags("-m32")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("Compiler = Clang/LLVM")
    if(BUILD_LIBS_32_BIT STREQUAL "Yes")
        append_build_flags("-m32")
    endif()
else()
    message(SEND_ERROR "Unknown (unsupported) compiler detected. Supported is only GCC (on Linux) and MSVC (on Windows).")
endif()

message("Searching for Boost library ...")
set(Boost_USE_STATIC_LIBS   ON)
find_package(Boost REQUIRED COMPONENTS filesystem)
message("Boost STATUS:
    Includes ${Boost_INCLUDE_DIRS}
    Libs ${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})
set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH "${Boost_LIBRARIES}")

find_package(LLVM REQUIRED CONFIG)
llvm_map_components_to_libnames(LLVM_LIBS Core Passes)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LT_LLVM_INSTALL_DIR}")
if(NOT "${LLVM_VERSION_MAJOR}" VERSION_GREATER_EQUAL "10")
    message(WARNING "LLVM versions below 10 (found ${LLVM_VERSION_MAJOR}) are not supported.")
endif()

message("LLVM STATUS:
    Definitions ${LLVM_DEFINITIONS}
    Includes    ${LLVM_INCLUDE_DIRS}
    Libraries   ${LLVM_LIBRARY_DIRS}
    Targets     ${LLVM_TARGETS_TO_BUILD}")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/dist" CACHE STRING "Install path" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

set(CMAKE_INSTALL_RPATH "./")

set(3RD_INCLUDE_DIR "${CMAKE_TOOLCHAIN_FILE}/../../../installed/${VCPKG_TARGET_TRIPLET}/include")
cmake_path(NORMAL_PATH 3RD_INCLUDE_DIR)
include_directories(${3RD_INCLUDE_DIR})
message("3rd includes directory = ${3RD_INCLUDE_DIR}")
message("Sources directory = ${PROJECT_SOURCE_DIR}/src")
message("Install directory = ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_subdirectory(./src)

if(BUILD_LIBS_32_BIT STREQUAL "No")
    install(DIRECTORY ./benchmarks DESTINATION .)
    install(FILES ./LICENSE.txt DESTINATION .)
endif()

message("Generating build files ...")

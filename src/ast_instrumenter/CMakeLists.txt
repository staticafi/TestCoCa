set(THIS_TARGET_NAME ast_instrumenter)

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE RelWithDebInfo)
    set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO RelWithDebInfo)
    set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG Debug)
endif()

find_package(Clang REQUIRED CONFIG)

add_executable(${THIS_TARGET_NAME}
    include/ast_instrumenter/condition_instrumenter.hpp
    src/condition_instrumenter.cpp

    src/main.cpp
)

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

target_link_libraries(${THIS_TARGET_NAME}
    utility
    clang-cpp
    ${LLVM_LIBS}
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools")

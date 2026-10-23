set(THIS_TARGET_NAME ir_instrumenter)

add_executable(${THIS_TARGET_NAME} 
    include/ir_instrumenter/program_info.hpp
    src/program_info.cpp

    include/ir_instrumenter/program_options.hpp
    src/program_options.cpp

    include/ir_instrumenter/llvm_instrumenter.hpp
    src/llvm_instrumenter.cpp

    include/ir_instrumenter/branch_instrumenter.hpp
    src/branch_instrumenter.cpp

    include/ir_instrumenter/goal_instrumenter.hpp
    src/goal_instrumenter.cpp

    include/ir_instrumenter/error_instrumenter.hpp
    src/error_instrumenter.cpp

    include/ir_instrumenter/rename_only_instrumenter.hpp

    include/ir_instrumenter/instrumenter_factory.hpp
    src/instrumenter_factory.cpp

    src/main.cpp

    src/run.cpp
)

include_directories(${LLVM_INCLUDE_DIRS})

target_link_libraries(${THIS_TARGET_NAME}
    utility
    ${LLVM_LIBS}
    )


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/" OPTIONAL DESTINATION "tools" FILES_MATCHING PATTERN "*.dll")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Debug/" OPTIONAL DESTINATION "tools" FILES_MATCHING PATTERN "*.dll")

set(THIS_TARGET_NAME target)

add_library(${THIS_TARGET_NAME}
        include/target/target.hpp
        src/target.cpp

        include/target/instrumentation_types.hpp
        src/instrumentation_types.cpp

        src/instrumentation.cpp

        src/verifier_functions.cpp

        src/main.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

if(BUILD_LIBS_32_BIT STREQUAL "Yes")
    install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib32")
else()
    install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
endif()

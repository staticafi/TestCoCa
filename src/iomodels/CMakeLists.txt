set(THIS_TARGET_NAME iomodels)

add_library(${THIS_TARGET_NAME}
    ./include/iomodels/iomanager.hpp
    ./src/iomanager.cpp

        include/iomodels/input_model.hpp
        src/input_model.cpp

        ./include/iomodels/configuration.hpp
    ./src/configuration.cpp
        ../driver/src/run_analyzer.cpp
        ../driver/include/driver/run_analyzer.h
)

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

if(FIZZ_BUILD_LIBS_32_BIT STREQUAL "Yes")
    install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib32")
else()
    install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
endif()

set(THIS_TARGET_NAME client)

add_executable(${THIS_TARGET_NAME} 
    ./include/client/program_options.hpp
    ./src/program_options.cpp

    ./include/client/program_info.hpp
    ./src/program_info.cpp

    ./src/run.cpp
    ./src/main.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    connection
    instrumentation
    iomodels
    utility
    ${Boost_LIBRARIES}
    Threads::Threads
    )

if (LIBRT)
    target_link_libraries(${THIS_TARGET_NAME} ${LIBRT})
endif()


set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools")

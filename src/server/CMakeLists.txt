set(THIS_TARGET_NAME server) 

add_executable(${THIS_TARGET_NAME}
    include/server/program_info.hpp
    src/program_info.cpp

    include/server/program_options.hpp
    src/program_options.cpp

    src/main.cpp
    src/run.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    instrumentation
    connection
    iomodels
    instrumentation # Duplicating because of Mingw (otherwise linking errors)
    connection  # Duplicating because of Mingw (otherwise linking errors)
    utility
    Threads::Threads
    ${Boost_LIBRARIES}
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

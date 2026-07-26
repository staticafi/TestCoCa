set(THIS_TARGET_NAME driver)

add_executable(${THIS_TARGET_NAME}
    include/driver/program_info.hpp
    src/program_info.cpp

    include/driver/program_options.hpp
    src/program_options.cpp

    src/main.cpp

    include/driver/test_parser.hpp
    src/test_parser.cpp

    include/driver/run_analyzer.hpp
    src/run_analyzer.cpp

    include/driver/wave_scheduler.hpp
    src/wave_scheduler.cpp

    include/driver/result_writer.hpp
    src/result_writer.cpp

    include/driver/test_runner.hpp
    src/test_runner.cpp
)

target_link_libraries(${THIS_TARGET_NAME}
    target
    connection
    iomodels
    utility
    ${Boost_LIBRARIES}
    )

if (LIBRT)
    target_link_libraries(${THIS_TARGET_NAME} ${LIBRT})
endif()

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools")

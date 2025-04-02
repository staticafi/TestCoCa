include_directories(
    "${PROJECT_SOURCE_DIR}/src/target/include"
    "${PROJECT_SOURCE_DIR}/src/instrumenter/include"
    "${PROJECT_SOURCE_DIR}/src/connection/include"
    "${PROJECT_SOURCE_DIR}/src/iomodels/include"
    "${PROJECT_SOURCE_DIR}/src/utility/include"
    "${PROJECT_SOURCE_DIR}/src/driver/include"
    )

message("Including the following libraries to the build:")

message("iomodels")
add_subdirectory(./iomodels)

message("connection")
add_subdirectory(./connection)

message("target")
add_subdirectory(./target)


if(BUILD_LIBS_32_BIT STREQUAL "No")
    message("instrumenter")
    add_subdirectory(./instrumenter)

    message("driver")
    add_subdirectory(./driver)

    message("runner")
    add_subdirectory(./runner)

    set(LIBUTILITY_INSTALL_DIR "lib")
else()
    set(LIBUTILITY_INSTALL_DIR "lib32")
endif()

message("utility")
add_subdirectory(./utility)

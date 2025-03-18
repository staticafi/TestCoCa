include_directories(
    "target/include"
    "${PROJECT_SOURCE_DIR}/src/instrumenter/include"
    "${PROJECT_SOURCE_DIR}/src/connection/include"
    "${PROJECT_SOURCE_DIR}/src/iomodels/include"
    "${PROJECT_SOURCE_DIR}/src/utility/include"
    "${PROJECT_SOURCE_DIR}/src/driver/include"
    )

message("Including the following libraries to the build:")

message("       utility")
add_subdirectory(./utility)

message("       iomodels")
add_subdirectory(./iomodels)

message("       connection")
add_subdirectory(./connection)

message("target")
add_subdirectory(target)

message("       instrumenter")
add_subdirectory(./instrumenter)

message("       driver")
add_subdirectory(driver)

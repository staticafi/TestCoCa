include_directories(
    "${PROJECT_SOURCE_DIR}/src/instrumentation/include"
    "${PROJECT_SOURCE_DIR}/src/instrumenter/include"
    "${PROJECT_SOURCE_DIR}/src/connection/include"
    "${PROJECT_SOURCE_DIR}/src/iomodels/include"
    "${PROJECT_SOURCE_DIR}/src/utility/include"
    "${PROJECT_SOURCE_DIR}/src/client/include"
    )

message("Including the following libraries to the build:")

message("       utility")
add_subdirectory(./utility)

message("       iomodels")
add_subdirectory(./iomodels)

message("       connection")
add_subdirectory(./connection)

message("       instrumentation")
add_subdirectory(./instrumentation)

message("       client")
add_subdirectory(./client)

message("       instrumenter")
add_subdirectory(./instrumenter)

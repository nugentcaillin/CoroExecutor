
# installation

This library can be made available using FetchContent as follows in your CMakeLists.txt
```console
include(FetchContent)
FetchContent_Declare(
    CoroExecutor
    GIT_REPOSITORY https://github.com/nugentcaillin/CoroExecutor.git
    GIT_TAG e5198c745066d6abf96002ee8c9b212de7abf9b8
)
FetchContent_MakeAvailable(CoroExecutor)

target_link_libraries(${PROJECT_NAME} PRIVATE CoroExecutor)
```
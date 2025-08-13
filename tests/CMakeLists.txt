cmake_minimum_required(VERSION 3.15)

project(tests)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

list(APPEND CMAKE_PREFIX_PATH "../install")
find_package(CoroExecutor CONFIG REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE tests.cpp)



target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    CoroExecutor::CoroExecutor
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

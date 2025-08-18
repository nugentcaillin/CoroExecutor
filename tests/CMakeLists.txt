cmake_minimum_required(VERSION 3.15)

project(tests)

# google test has some old versions of CMake
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)
FetchContent_MakeAvailable(googletest)

enable_testing()

#list(APPEND CMAKE_PREFIX_PATH "../")
#find_package(CoroExecutor CONFIG REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} 
  PRIVATE
  testing.hpp 
  LifetimeManagedCoroutineTest.cpp
  CoroExecutorTest.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -g)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    CoroExecutor
    GTest::gtest_main 
    #-fsanitize=address
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

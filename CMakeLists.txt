cmake_minimum_required(VERSION 3.15)

set(version "1.0.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# defaults
if (NOT DEFINED BUILD_TESTS)
    message(STATUS "BUILD_TESTS not set, defaulting to False")
    set(BUILD_TESTS False)
else()
    message(STATUS "BUILD_TESTS set to ${BUILD_TESTS}")
endif()


project(CoroExecutor
    VERSION ${version}
)

add_library(${PROJECT_NAME} STATIC)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_sources(${PROJECT_NAME} 
    PRIVATE
    src/CoroExecutor.cpp 
    src/LifetimeManagedCoroutine.cpp
)


target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (DEFINED CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(
        STATUS 
        "CMAKE_INSTALL_PREFIX not set\n"
        "default: ${CMAKE_INSTALL_PREFIX}\n"
        "setting to: ${CMAKE_SOURCE_DIR}/build/install"
    )
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_SOURCE_DIR}/build/install"
        CACHE PATH "where library is installed" FORCE
    )
else()
    message(
        STATUS 
        "CMAKE_INSTALL PREFIX already set to ${CMAKE_INSTALL_PREFIX}"
    )
endif() 

set(public_headers
    include/CoroExecutor/CoroExecutor.hpp
)


include (GNUInstallDirs)


set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${public_headers}")

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

install(TARGETS ${PROJECT_NAME}
    EXPORT "${PROJECT_NAME}Targets"
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT "${PROJECT_NAME}Targets"
    FILE "${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${PROJECT_NAME}:: # could be wrong
    DESTINATION cmake
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION "${version}"
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION cmake 
)

install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION cmake
)



if (BUILD_TESTS)
    target_compile_options(${PROJECT_NAME} PRIVATE -g)
    add_subdirectory(tests)
endif()

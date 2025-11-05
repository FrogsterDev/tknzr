cmake_minimum_required(VERSION 3.20)
project(tknzr LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(tknzr src/tknzr.cpp)
add_library(tknzr::tknzr ALIAS tknzr)

target_include_directories(tknzr
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(tknzr PUBLIC cxx_std_20)

# --- Installation ---
include(GNUInstallDirs)

install(
    TARGETS tknzr
    EXPORT tknzrTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets so find_package() works
install(EXPORT tknzrTargets
    FILE tknzrTargets.cmake
    NAMESPACE tknzr::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tknzr
)

# Generate and install a Config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
    cmake/tknzrConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tknzrConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tknzr
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tknzrConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tknzr
)

# --- Tests ---
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(test_tknzr tests/test_tknzr.cpp)
    target_link_libraries(test_tknzr PRIVATE tknzr::tknzr GTest::gtest_main)
    add_test(NAME tknzr_test COMMAND test_tknzr)
endif()
cmake_minimum_required(VERSION 3.28)

project(test_gps_nmea C CXX ASM)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts -fcommon")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-allow-multiple-definition")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(${CMAKE_PROJECT_NAME}_nmea)
target_sources(${CMAKE_PROJECT_NAME}_nmea
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/nmea/nmea.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test_gps_nmea_lib.cpp
)
target_sources(${CMAKE_PROJECT_NAME}_nmea
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/nmea/nmea_iface.cpp
)

target_link_libraries( ${CMAKE_PROJECT_NAME}_nmea
        GTest::gtest_main
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(${CMAKE_PROJECT_NAME}_nmea)

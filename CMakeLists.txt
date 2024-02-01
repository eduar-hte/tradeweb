cmake_minimum_required(VERSION 3.18)

project(tradeweb)

enable_testing()
find_package(GTest REQUIRED)

add_executable(OrderCacheTests OrderCacheTests.cpp OrderCache.cpp)

target_link_libraries(OrderCacheTests GTest::GTest GTest::Main)

add_test(OrderCacheTests OrderCacheTests)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET OrderCacheTests PROPERTY CXX_STANDARD 17)
endif()
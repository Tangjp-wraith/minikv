cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SkipList
  DESCRIPTION "SkipList"
  LANGUAGES C CXX
)

set(SKIPLIST_BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build")
set(SKIPLIST_CLANG_SEARCH_PATH "/usr/local/bin" "/usr/bin")
set(SKIPLIST_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

include_directories(${SKIPLIST_SRC_INCLUDE_DIR})
find_package(Threads)
find_package(GTest REQUIRED)

# ############ skiplist_test #############
add_executable(test_SkipList
  test/SkipList_test.cc
)

target_link_libraries(test_SkipList
  GTest::GTest
  GTest::Main
)

# ########### threadpool_test ############
add_executable(test_ThreadPool
  test/ThreadPool_test.cc
)

target_link_libraries(test_ThreadPool
  GTest::GTest
  GTest::Main
)

############ server #####################
add_executable(server
  src/Server.cc
  src/main.cc
)

target_link_libraries(server
  ${CMAKE_THREAD_LIBS_INIT}
)
############ client #####################
add_executable(client
  src/Client.cc
)

target_link_libraries(client
  ${CMAKE_THREAD_LIBS_INIT}
)
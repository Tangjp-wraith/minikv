cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(kvDB
  DESCRIPTION "kvDB"
  LANGUAGES C CXX
)

set(KVDB_BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build")
set(KVDB_CLANG_SEARCH_PATH "/usr/local/bin" "/usr/bin")
set(KVDB_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

include_directories(${KVDB_SRC_INCLUDE_DIR})
find_package(Threads)
find_package(GTest REQUIRED)

# ############ main #############
add_executable(main
  test/main.cc
)

target_link_libraries(main
  ${CMAKE_THREAD_LIBS_INIT}
)

# ############ stress_test #############
# add_executable(test_stress
#   test/stress_test.cc
# )

# target_link_libraries(test_stress
#   ${CMAKE_THREAD_LIBS_INIT}
# )
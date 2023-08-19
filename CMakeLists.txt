cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(kvDB
  DESCRIPTION "minikv"
  LANGUAGES C CXX
)

set(KV_BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build")
set(KV_CLANG_SEARCH_PATH "/usr/local/bin" "/usr/bin")
set(KV_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/base)

include_directories(${KV_SRC_INCLUDE_DIR})
find_package(Threads)
find_package(GTest REQUIRED)

# ############ main #############
add_executable(main
  test/main.cc
)

target_link_libraries(main
  ${CMAKE_THREAD_LIBS_INIT}
)

# ############ test #############
add_subdirectory(test)

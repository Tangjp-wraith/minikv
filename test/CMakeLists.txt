project(test)

add_executable(test_arena test_arena.cc ../base/arena.h ../base/arena.cc ../base/random.h)

target_link_libraries(test_arena
  GTest::GTest
  GTest::Main
)
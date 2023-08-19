#include <gtest/gtest.h>

#include "../base/arena.h"
#include "../base/random.h"

TEST(TestArena, allocateTest) {
  Arena arena;
  // 1.分配小于1k 只分配一次4096+8字节大的内存 8字节代表指针
  arena.Allocate(1024);
  EXPECT_EQ(arena.MemoryUsage(), 4096 + 8);  // 4104
  arena.Allocate(1024);
  EXPECT_EQ(arena.MemoryUsage(), 4096 + 8);
  arena.Allocate(1024);
  EXPECT_EQ(arena.MemoryUsage(), 4096 + 8);
  // 3072 + 800 = 3872
  arena.Allocate(800);
  EXPECT_EQ(arena.MemoryUsage(), 4096 + 8);
  EXPECT_EQ(arena.GetRemainBytes(), 4096 - 3872);  // 224
  EXPECT_EQ(arena.GetBlocksSize(), 1);
  // 2.分配大于剩余内存 且 小于等于1k 浪费掉了224字节内存
  arena.Allocate(1024);  // 4104
  // 8208 4104 + 4104(新分配) = 8208
  EXPECT_EQ(arena.MemoryUsage(), 4104 + 4104);
  // 剩余3072 4096-1024=3072
  EXPECT_EQ(arena.GetRemainBytes(), 4096 - 1024);
  EXPECT_EQ(arena.GetBlocksSize(), 2);
  arena.Allocate(2050);
  // 剩余1022 4096-3074=1022
  EXPECT_EQ(arena.GetRemainBytes(), 4096 - 3074);
  EXPECT_EQ(arena.MemoryUsage(), 4104 + 4104);
  EXPECT_EQ(arena.GetBlocksSize(), 2);
  // 3.当前剩余块内存不足1k且申请大于1k
  arena.Allocate(1025);  // 重新分配1025+8=1033
  // 无变化 仍旧是上次块的指向 1022
  EXPECT_EQ(arena.GetRemainBytes(), 4096 - 3074);
  // 9241 8208 + 1033 = 9241
  EXPECT_EQ(arena.MemoryUsage(), 8208 + 1033);
  EXPECT_EQ(arena.GetBlocksSize(), 3);
}

TEST(TestArena, SimpleTest) {
  std::vector<std::pair<size_t, char*> > allocated;
  Arena arena;
  const int N = 100000;
  size_t bytes = 0;
  Random rnd(301);
  for (int i = 0; i < N; i++) {
    size_t s;
    if (i % (N / 10) == 0) {
      s = i;
    } else {
      s = rnd.OneIn(4000)
              ? rnd.Uniform(6000)
              : (rnd.OneIn(10) ? rnd.Uniform(100) : rnd.Uniform(20));
    }
    if (s == 0) {
      // Our arena disallows size 0 allocations.
      s = 1;
    }
    char* r;
    if (rnd.OneIn(10)) {
      r = arena.AllocateAligned(s);
    } else {
      r = arena.Allocate(s);
    }

    for (size_t b = 0; b < s; b++) {
      // Fill the "i"th allocation with a known bit pattern
      r[b] = i % 256;
    }
    bytes += s;
    allocated.push_back(std::make_pair(s, r));
    ASSERT_GE(arena.MemoryUsage(), bytes);
    if (i > N / 10) {
      ASSERT_LE(arena.MemoryUsage(), bytes * 1.10);
    }
  }
  for (size_t i = 0; i < allocated.size(); i++) {
    size_t num_bytes = allocated[i].first;
    const char* p = allocated[i].second;
    for (size_t b = 0; b < num_bytes; b++) {
      // Check the "i"th allocation for the known bit pattern
      ASSERT_EQ(int(p[b]) & 0xff, i % 256);
    }
  }
}

#include <quadtree/FastArray.h>

#include <gtest/gtest.h>

namespace light::test
{

using Int = uint32_t;
TEST(FastArrayTests, BuildEmpty)
{
    FastArray<Int> a;
    EXPECT_EQ(a.size(), 0);
    EXPECT_TRUE(a.empty());
}

TEST(FastArrayTests, PushBack)
{
    FastArray<int> a;
    EXPECT_EQ(a.size(), 0);
    a.push_back(5);
    EXPECT_EQ(a.size(), 1);
    EXPECT_EQ(a[0], 5);
    EXPECT_FALSE(a.empty());
}

TEST(FastArrayTests, PushBackMany)
{
    FastArray<Int> a;

    for (size_t i = 0; i < 1024; ++i)
    {
        a.push_back(i);
    }
    EXPECT_EQ(a.size(), 1024);

    const FastArray<Int>& constArray = a;
    for (size_t i = 0; i < 1024; ++i)
    {
        EXPECT_EQ(a[i], i);
        EXPECT_EQ(constArray[i], i);
    }
}

TEST(FastArrayTests, PeekPop)
{
    FastArray<Int> a;

    for (size_t i = 1; i <= 1024; ++i)
    {
        a.push_back(i);
    }
    EXPECT_EQ(a.size(), 1024);

    for (size_t i = 1024; i >= 1; --i)
    {
        EXPECT_EQ(a.peek(), i);
        EXPECT_EQ(a.size(), i);
        EXPECT_EQ(a.pop(), i);
        EXPECT_EQ(a.size(), i - 1);
    }

    EXPECT_TRUE(a.empty());
}

}

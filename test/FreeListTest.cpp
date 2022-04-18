#include <light/FreeList.h>

#include <gtest/gtest.h>

namespace light::test
{
using MyInt = uint32_t;
TEST(FreeListTests, BuildEmpty)
{
    FreeList<MyInt> a;
    EXPECT_EQ(a.size(), 0);
    EXPECT_EQ(a.range(), 0);
}

TEST(FreeListTests, PushBack)
{
    FreeList<MyInt> a;
    const auto i1 = a.push_back(1);
    EXPECT_EQ(i1, 0);
    const auto i2 = a.push_back(2);
    EXPECT_EQ(i2, 1);
    const auto i3 = a.push_back(3);
    EXPECT_EQ(i3, 2);
    const auto i4 = a.push_back(4);
    EXPECT_EQ(i4, 3);
    const auto i5 = a.push_back(5);
    EXPECT_EQ(i5, 4);

    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);
    EXPECT_EQ(5, a[4]);

    EXPECT_EQ(a.size(), 5);
    EXPECT_EQ(a.range(), 5);
}

TEST(FreeListTests, Erase)
{
    FreeList<MyInt> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(4);
    a.push_back(5);

    a.erase(0);

    // we deleted element 0, not it points to the next free element in array.
    // since it's the first and last free element, it points to nowhere.
    EXPECT_EQ(std::numeric_limits<uint32_t>::max(), a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);
    EXPECT_EQ(5, a[4]);

    EXPECT_EQ(a.size(), 4);
    EXPECT_EQ(a.range(), 5);
}

TEST(FreeListTests, RemoveAdd)
{
    FreeList<MyInt> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(4);
    a.push_back(5);

    a.erase(0);
    a.push_back(99);

    EXPECT_EQ(99, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);
    EXPECT_EQ(4, a[3]);
    EXPECT_EQ(5, a[4]);
}

TEST(FreeListTests, RemoveAddSeveral)
{
    FreeList<MyInt> a;
}

}

#include <quadtree/Quadtree.h>

#include <gtest/gtest.h>

namespace light::test
{
constexpr auto Id = 0;

TEST(QuadtreeTests, EmptyTest)
{
    Quadtree quadtree(Point(0, 0), Point(1, 1));
    EXPECT_EQ(quadtree.size(), 0);
}

TEST(QuadtreeTests, InsertOne)
{
    Quadtree quadtree(Point(0, 0), Point(1, 1));
    //quadtree.insert(Point(0.2, 0.2), Point(0.3, 0.3), Id);
    //EXPECT_EQ(quadtree.size(), 1);
}

// TEST(QuadtreeTests, SubdivideFirstQuad)
// TEST(QuadtreeTests, MaxDepth)
// TEST(QuadtreeTests, MaxChildren)
// TEST(QuadtreeTests, InsertMany)
// TEST(QuadtreeTests, Remove)
// TEST(QuadtreeTests, Cleanup)

}

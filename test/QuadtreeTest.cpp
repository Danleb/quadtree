#include <light/Quadtree.h>

#include <gtest/gtest.h>

namespace light::test
{

TEST(RectanglesOverlapTest, SameRectangle_1)
{
    Point bottomLeft{ 0, 0 };
    Point topRight{ 1, 1 };
    EXPECT_TRUE(isRectanglesOverlap(bottomLeft, topRight, bottomLeft, topRight));
}

TEST(RectanglesOverlapTest, SameRectangle_2)
{
    Point bottomLeft{ 0.11, 0.22 };
    Point topRight{ 1.11, 1.22 };
    EXPECT_TRUE(isRectanglesOverlap(bottomLeft, topRight, bottomLeft, topRight));
}

TEST(RectanglesOverlapTest, SameRectangle_3)
{
    Point bottomLeft{ -10, -10 };
    Point topRight{ 10, 10 };
    EXPECT_TRUE(isRectanglesOverlap(bottomLeft, topRight, bottomLeft, topRight));
}

TEST(RectanglesOverlapTest, HalfOverlap)
{
    Point rect1_bottomLeft{ 0, 0 };
    Point rect1_topRight{ 10, 10 };
    Point rect2_bottomLeft{ 5, 5 };
    Point rect2_topRight{ 15, 15 };
    EXPECT_TRUE(
      isRectanglesOverlap(rect1_bottomLeft, rect1_topRight, rect2_bottomLeft, rect2_topRight));
}

TEST(RectanglesOverlapTest, BottomSide1IsHigherThanTopSide2)
{
    Point rect1_bottomLeft{ 0, 0 };
    Point rect1_topRight{ 10, 10 };
    Point rect2_bottomLeft{ 10, 10.1 };
    Point rect2_topRight{ 10, 20 };
    EXPECT_FALSE(
      isRectanglesOverlap(rect1_bottomLeft, rect1_topRight, rect2_bottomLeft, rect2_topRight));
}

TEST(RectanglesOverlapTest, SmallOverlap)
{
    Point rect1_bottomLeft{ 0, 0 };
    Point rect1_topRight{ 10, 10 };
    Point rect2_bottomLeft{ -10, -10 };
    Point rect2_topRight{ 0.1, 0.1 };
    EXPECT_FALSE(
      isRectanglesOverlap(rect1_bottomLeft, rect1_topRight, rect2_bottomLeft, rect2_topRight));
}

TEST(QuadtreeTests, EmptyTest)
{
    Quadtree quadtree(Point(0, 0), Point(1, 1));
    EXPECT_EQ(quadtree.size(), 0);
}

TEST(QuadtreeTests, InsertOne)
{
    const Point areaBottomLeft{ 0, 0 };
    const Point areaTopRight{ 1, 1 };
    Quadtree quadtree{ areaBottomLeft, areaTopRight };
    quadtree.insert(Point(0.2, 0.2), Point(0.3, 0.3), Id(1));
    EXPECT_EQ(quadtree.size(), 1);

    int counter = 0;
    quadtree.forEachObjectInArea(areaBottomLeft,
                                 areaTopRight,
                                 [&](const Id& id, Point bottomLeft, Point topRight)
                                 { ++counter; });
    EXPECT_EQ(counter, 1);

    counter = 0;
    quadtree.forEachObjectInArea(
      { 0.5, 0.5 }, { 2, 2 }, [&](const Id& id, Point bottomLeft, Point topRight) { ++counter; });
    EXPECT_EQ(counter, 0);
}

// TEST(QuadtreeTests, SubdivideFirstQuad)
// TEST(QuadtreeTests, MaxDepth)
// TEST(QuadtreeTests, MaxChildren)
// TEST(QuadtreeTests, InsertMany)
// TEST(QuadtreeTests, Remove)
// TEST(QuadtreeTests, Cleanup)
}

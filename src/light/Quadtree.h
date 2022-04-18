#pragma once

#include <light/FastArray.h>
#include <light/FreeList.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <functional>
#include <queue>
#include <stack>

namespace light
{
using Point = glm::vec2;
using Id = uint32_t;

// Representation of actual value we want to store.
struct QuadElement
{
    // Outer Id of the element.
    Id id;

    // Extents of the current element.
    Point bottomLeft;
    Point topRight;
};

// Represents a reference to QuadElement.
struct QuadElementNode
{
    // Points to the next QuadElementNode in the leaf node.
    // A value of -1 indicates the end of the list.
    uint32_t next;

    // Stores the index of the QuadElement.
    uint32_t quadElementIndex;
};

struct QuadNode
{
    // Points to the first child (QuadNode) if this node is a branch or the first
    // element (QuadElementNode) if this node is a leaf.
    // This is index of .
    uint32_t firstChild;

    // Stores the number of elements in the leaf or NIL if it this node is
    // not a leaf.
    uint32_t count;

    inline bool isBranch() const { return count == NIL; }

    inline bool isLeaf() const { return !isBranch(); }
};

bool isRectanglesOverlap(light::Point rectBottomLeft1,
                         light::Point rectTopRight1,
                         light::Point rectBottomLeft2,
                         light::Point rectTopRight2);

class Quadtree
{
public:
    /**
     * @brief Constructs an empty Quadtree for specified 2D area.
     * @param areaBottomLeft Bottom left corner of work area.
     * @param areaTopRight Top right corner of work area.
     * @param maxElementsPerNode Maximum amount of elements that can be stored in quad node before
     * it will be splitted. Quad won't be splitted anymore when maxDepth is reached.
     * @param maxDepth Max depth of nested quad nodes.
     */
    Quadtree(Point areaBottomLeft,
             Point areaTopRight,
             int maxElementsPerNode = 8,
             int maxDepth = 8);

    size_t size() const;

    void reserve(size_t capacity);

    bool insert(Point rectBottomLeft, Point rectTopRight, Id id);

    void remove(uint32_t index);

    using IterateObjectsCallback =
      std::function<void(const Id& id, Point bottomLeft, Point topRight)>;

    void forEachObjectInArea(Point areaBottomLeft,
                             Point areaTopRight,
                             const IterateObjectsCallback& callback) const;

    using TraverseQuadCallback = std::function<void(const Point& bottomLeft, const Point& size)>;

    /**
     * @brief Function for traversing quads to visualize them.
     * @tparam QuadsObserver
     * @param quadsObserver
     */
    void traverseQuads(const TraverseQuadCallback& quadsObserver) const;

private:
    bool isValidRectangle(Point rectBottomLeft, Point rectTopRight) const;

    FreeList<QuadElement> m_elements;
    FreeList<QuadElementNode> m_elementNodes;
    FreeList<QuadNode> m_quadNodes;
    uint32_t m_freeNode;
    Point m_areaBottomLeft;
    Point m_areaTopRight;
    int m_maxElementsPerNode;
    int m_maxDepth;
};

}

#pragma once

#include <quadtree/FastArray.h>
#include <quadtree/FreeList.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <queue>
#include <stack>

namespace light
{
using Point = glm::vec2;

// Representation of actual value we want to store.
struct QuadElement
{
    // Outer Id of the element.
    uint32_t id;

    // Extents of the current element.
    Point bottomLeft;
    Point topRight;
};

// Represents an element node in the quadtree.
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

    bool isBranch() const { return count == NIL; }

    bool isLeaf() const { return !isBranch(); }
};

struct InsertData
{
    uint32_t elementIndex;
    uint32_t quadIndex;
    uint32_t depth;
    Point bottomLeftBound;
    Point size;
};

struct TraverseQuadData
{
    uint32_t quadIndex;
    Point bottomLeft;
    Point size;
};

class Quadtree
{
public:
    /// <summary>
    ///
    /// </summary>
    /// <param name="areaBottomLeft">Bottom left corner of work area.</param>
    /// <param name="areaTopRight">Top right corner of work area.</param>
    /// <param name="maxElementsPerNode">Maximum amount of elements that can be stored in quad node
    /// before it will be splitted. Quad won't be splitted anymore when maxDepth is reached.</param>
    /// <param name="maxDepth">Max depth of nested quad nodes.</param>
    Quadtree(Point areaBottomLeft, Point areaTopRight, int maxElementsPerNode = 8, int maxDepth = 8)
      : m_freeNode{ NIL }
      , m_areaBottomLeft{ areaBottomLeft }
      , m_areaTopRight{ areaTopRight }
      , m_maxElementsPerNode{ maxElementsPerNode }
      , m_maxDepth{ maxDepth }
    {
        QuadNode root;
        root.count = 0;
        root.firstChild = NIL;
        m_quadNodes.push_back(root);
    }

    bool insert(Point rectBottomLeft, Point rectTopRight, uint32_t id)
    {
        // if ill-formed rectangle
        if (rectBottomLeft.x > rectTopRight.x || rectBottomLeft.y > rectTopRight.y)
        {
            return false;
        }

        // if rectangle is outside of work area
        if (rectBottomLeft.x > m_areaTopRight.x || rectTopRight.x < m_areaBottomLeft.x ||
            rectBottomLeft.y > m_areaTopRight.y || rectTopRight.y < m_areaBottomLeft.y)
        {
            return false;
        }

        QuadElement quadElement;
        quadElement.id = id;
        quadElement.bottomLeft = rectBottomLeft;
        quadElement.topRight = rectTopRight;
        const auto elementIndex = m_elements.push_back(quadElement);

        /*
         * Cartestian coordinate system is used.
         * Order of the quadrants in the quadtree:
         * ┌───┬───┐
         * │ 1 │ 2 │
         * ├───┼───┤
         * │ 3 │ 4 │
         * └───┴───┘
         *
         */

        FastArray<InsertData> elementsToInsert;
        // at first, we want to insert our new element into root
        const auto rootSize = m_areaTopRight - m_areaBottomLeft;
        elementsToInsert.push_back({ elementIndex, 0, 0, m_areaBottomLeft, rootSize });

        while (!elementsToInsert.empty())
        {
            // Index of QuadElement we want to insert &&
            // Index of QuadNode we are working with
            const auto [currentElementIndex,
                        currentQuadIndex,
                        currentDepth,
                        currentBottomLeft,
                        currentSize] = elementsToInsert.pop();
            auto& currentQuad = m_quadNodes[currentQuadIndex];
            uint32_t currentQuadFirstChild = currentQuad.firstChild;

            if (currentQuad.isLeaf())
            {
                // if children count is less than max children count or we reached the max tree
                // depth
                // - we can just insert this element to current quadrant
                if (currentQuad.count < m_maxElementsPerNode || currentDepth == m_maxDepth)
                {
                    QuadElementNode quadElementNode;
                    quadElementNode.quadElementIndex = currentElementIndex;
                    quadElementNode.next = currentQuad.firstChild;
                    const auto newQuadElementNodeIndex = m_elementNodes.push_back(quadElementNode);
                    currentQuad.firstChild = newQuadElementNodeIndex;
                    ++currentQuad.count;
                    continue;
                }
                else
                {
                    // otherwise, subdivide, since max elements count is reached and max depth isn't
                    // reached. take out all elements of current node, subdivide it, then reinsert
                    // all elements again.

                    // push elements to reinsert
                    auto currentChildIndex = currentQuad.firstChild;
                    while (currentChildIndex != NIL)
                    {
                        InsertData insertData;
                        insertData.elementIndex =
                          m_elementNodes[currentChildIndex].quadElementIndex;
                        insertData.quadIndex = currentQuadIndex;
                        insertData.depth = currentDepth;
                        insertData.bottomLeftBound = currentBottomLeft;
                        insertData.size = currentSize;
                        elementsToInsert.push_back(insertData);
                        currentChildIndex = m_elementNodes[currentChildIndex].next;
                    }

                    // we turn current node into branch
                    currentQuad.count = NIL;

                    if (m_freeNode == NIL)
                    {
                        QuadNode emptyLeaf;
                        emptyLeaf.count = 0;
                        emptyLeaf.firstChild = NIL;

                        currentQuadFirstChild = m_quadNodes.size();
                        currentQuad.firstChild = currentQuadFirstChild;
                        m_quadNodes.push_back(emptyLeaf);
                        m_quadNodes.push_back(emptyLeaf);
                        m_quadNodes.push_back(emptyLeaf);
                        m_quadNodes.push_back(emptyLeaf);
                    }
                    else
                    {
                        // todo
                        // currentQuad.firstChild = m_freeNode;
                        return false;
                    }
                }
            }

            const auto newSize = currentSize * 0.5f;
            const auto currentCenter = currentBottomLeft + newSize;
            const auto newDepth = currentDepth + 1;

            // Since element we want to insert is a rectangle (not a point), it may overlap several
            // quadrants. In such case, we will insert it in all overlapped quadrants.
            InsertData subQuadData;
            subQuadData.depth = newDepth;
            subQuadData.elementIndex = currentElementIndex;
            subQuadData.size = newSize;

            const auto& currentElement = m_elements[currentElementIndex];

            if (currentElement.bottomLeft.x < currentCenter.x &&
                currentElement.topRight.y > currentCenter.y)
            {
                // quadrant #1
                const auto quad1BottomLeft = currentBottomLeft + Point(0, newSize.y);
                subQuadData.bottomLeftBound = quad1BottomLeft;
                subQuadData.quadIndex = currentQuadFirstChild + 0;
                elementsToInsert.push_back(subQuadData);
            }
            if (currentElement.topRight.x > currentCenter.x &&
                currentElement.topRight.y > currentCenter.y)
            {
                // quadrant #2;
                subQuadData.bottomLeftBound = currentCenter;
                subQuadData.quadIndex = currentQuadFirstChild + 1;
                elementsToInsert.push_back(subQuadData);
            }
            if (currentElement.bottomLeft.x < currentCenter.x &&
                currentElement.bottomLeft.y < currentCenter.y)
            {
                // quadrant #3
                subQuadData.bottomLeftBound = currentBottomLeft;
                subQuadData.quadIndex = currentQuadFirstChild + 2;
                elementsToInsert.push_back(subQuadData);
            }
            if (currentElement.topRight.x > currentCenter.x &&
                currentElement.bottomLeft.y < currentCenter.y)
            {
                // quadrant #4
                const auto quad4BottomLeft = currentBottomLeft + Point(newSize.x, 0);
                subQuadData.bottomLeftBound = quad4BottomLeft;
                subQuadData.quadIndex = currentQuadFirstChild + 3;
                elementsToInsert.push_back(subQuadData);
            }
        }

        return true;
    }

    void remove(uint32_t index)
    {
        //
        // m_freeNode = index;
    }

    template<typename NeightborsObserver>
    void traverse(Point bottomLeftBound, Point topRightBound, NeightborsObserver observer) const
    {
        //
    }

    /// <summary>
    /// Function for traversing quads to visualize them.
    /// </summary>
    /// <typeparam name="QuadsObserver"></typeparam>
    template<typename QuadsObserver>
    void traverseQuads(QuadsObserver quadsObserver)
    {
        std::queue<TraverseQuadData> quads;
        auto rootSize = m_areaTopRight - m_areaBottomLeft;
        quads.push({ 0, m_areaBottomLeft, rootSize });

        while (!quads.empty())
        {
            const auto [quadIndex, bottomLeft, size] = quads.front();
            quads.pop();
            const auto& quad = m_quadNodes[quadIndex];

            quadsObserver(bottomLeft, size);

            const auto newSize = size * 0.5f;

            if (quad.isBranch())
            {
                quads.push({ quad.firstChild + 0, bottomLeft + Point(0, newSize.y), newSize });
                quads.push({ quad.firstChild + 1, bottomLeft + newSize, newSize });
                quads.push({ quad.firstChild + 2, bottomLeft, newSize });
                quads.push({ quad.firstChild + 3, bottomLeft + Point(newSize.x, 0), newSize });
            }
        }
    }

    size_t size() const { return m_elements.size(); }

private:
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

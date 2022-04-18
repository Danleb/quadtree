#include "Quadtree.h"

namespace light
{

namespace
{
struct InsertData
{
    uint32_t elementIndex;
    uint32_t quadIndex;
    uint32_t depth;
    light::Point bottomLeftBound;
    light::Point size;
};

struct TraverseQuadData
{
    uint32_t quadIndex;
    light::Point bottomLeft;
    light::Point size;
};

}

const float EPS = 1e-5;

bool isRectanglesOverlap(light::Point rectBottomLeft1,
                         light::Point rectTopRight1,
                         light::Point rectBottomLeft2,
                         light::Point rectTopRight2)
{
    return rectTopRight2.x > rectBottomLeft1.x && rectTopRight2.y > rectBottomLeft1.y &&
           rectBottomLeft2.x < rectTopRight1.x && rectBottomLeft2.y < rectTopRight1.y;
}

Quadtree::Quadtree(Point areaBottomLeft, Point areaTopRight, int maxElementsPerNode, int maxDepth)
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

size_t Quadtree::size() const
{
    return m_elements.size();
}

void Quadtree::reserve(size_t capacity)
{
    // m_elements.reserve(capacity);
    // m_elementNodes.reserve(capacity * 2);

    //// suppose uniform element distribution per area
    // const auto estimatedLeafQuadsCount = capacity / m_maxElementsPerNode;
    // constexpr int SUBDIVISION_COUNT = 4;
    // auto estimatedDepth = log(estimatedLeafQuadsCount) / log(SUBDIVISION_COUNT);
    // if (estimatedDepth > m_maxDepth)
    //{
    //    estimatedDepth = m_maxDepth;
    //}
    // const auto estimatedQuadsCount = pow(SUBDIVISION_COUNT, estimatedDepth);
    // m_quadNodes.reserve(estimatedQuadsCount);
}

bool Quadtree::insert(Point rectBottomLeft, Point rectTopRight, Id id)
{
    if (!isValidRectangle(rectBottomLeft, rectTopRight))
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
                    insertData.elementIndex = m_elementNodes[currentChildIndex].quadElementIndex;
                    insertData.quadIndex = currentQuadIndex;
                    insertData.depth = currentDepth;
                    insertData.bottomLeftBound = currentBottomLeft;
                    insertData.size = currentSize;
                    elementsToInsert.push_back(insertData);
                    const auto nextChildIndex = m_elementNodes[currentChildIndex].next;
                    m_elementNodes.erase(currentChildIndex);
                    currentChildIndex = nextChildIndex;
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

void Quadtree::remove(uint32_t index)
{
    //
}

void Quadtree::forEachObjectInArea(Point rectBottomLeft,
                                   Point rectTopRight,
                                   const IterateObjectsCallback& callback) const
{
    if (!isValidRectangle(rectBottomLeft, rectTopRight))
    {
        return;
    }

    FastArray<TraverseQuadData> quadsToCheck;
    quadsToCheck.push_back({ 0, m_areaBottomLeft, m_areaTopRight - m_areaBottomLeft });

    while (!quadsToCheck.empty())
    {
        const auto currentTraverseData = quadsToCheck.pop();
        const auto& currentParentQuad = m_quadNodes[currentTraverseData.quadIndex];

        if (currentParentQuad.isLeaf())
        {
            // iterate over values
            auto quadElementNode = currentParentQuad.firstChild;

            while (quadElementNode != NIL)
            {
                const auto& currentQuadNode = m_elementNodes[quadElementNode];
                const auto& element = m_elements[currentQuadNode.quadElementIndex];

                if (isRectanglesOverlap(
                      rectBottomLeft, rectTopRight, element.bottomLeft, element.topRight))
                {
                    callback(element.id, element.bottomLeft, element.topRight);
                }

                quadElementNode = currentQuadNode.next;
            }
        }
        else
        {
            // it's a branch, add to stack quads that overlaps with target area

            const auto currentQuadFirstChild = currentParentQuad.firstChild;
            const auto subQuadSize = currentTraverseData.size * 0.5f;
            const auto currentCenter = currentTraverseData.bottomLeft + subQuadSize;
            const auto currentBottomLeft = currentTraverseData.bottomLeft;

            TraverseQuadData subQuadData;
            subQuadData.size = subQuadSize;

            if (rectBottomLeft.x < currentCenter.x && rectTopRight.y > currentCenter.y)
            {
                // quadrant #1
                const auto quad1BottomLeft = currentBottomLeft + Point(0, subQuadSize.y);
                subQuadData.bottomLeft = quad1BottomLeft;
                subQuadData.quadIndex = currentQuadFirstChild + 0;
                quadsToCheck.push_back(subQuadData);
            }
            if (rectTopRight.x > currentCenter.x && rectTopRight.y > currentCenter.y)
            {
                // quadrant #2;
                subQuadData.bottomLeft = currentCenter;
                subQuadData.quadIndex = currentQuadFirstChild + 1;
                quadsToCheck.push_back(subQuadData);
            }
            if (rectBottomLeft.x < currentCenter.x && rectBottomLeft.y < currentCenter.y)
            {
                // quadrant #3
                subQuadData.bottomLeft = currentBottomLeft;
                subQuadData.quadIndex = currentQuadFirstChild + 2;
                quadsToCheck.push_back(subQuadData);
            }
            if (rectTopRight.x > currentCenter.x && rectBottomLeft.y < currentCenter.y)
            {
                // quadrant #4
                const auto quad4BottomLeft = currentBottomLeft + Point(subQuadSize.x, 0);
                subQuadData.bottomLeft = quad4BottomLeft;
                subQuadData.quadIndex = currentQuadFirstChild + 3;
                quadsToCheck.push_back(subQuadData);
            }
        }
    }
}

void Quadtree::traverseQuads(const TraverseQuadCallback& quadsObserver) const
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

bool Quadtree::isValidRectangle(Point rectBottomLeft, Point rectTopRight) const
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

    return true;
}

}

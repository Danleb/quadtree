#pragma once

#include <light/Quadtree.h>

#include <functional>
#include <unordered_map>
#include <vector>

namespace light
{

using Vector2d = glm::vec2;

struct CircleData
{
    float speed;
    Point position;
    Vector2d movementDirection;
};

/**
 * @brief Class for simple 2d rigid body circles simulation.
 */
class CirclesSimulation
{
public:
    CirclesSimulation(const Point& bottomLeft,
                      const Point& topRight,
                      size_t circlesCount,
                      float circleRadius,
                      float speed);

    size_t size() const;

    /**
     * @brief Simulate a simulation step with updating all circles positions.
     * @param timeDelta Time delta in seconds passed since previous simulation step.
     */
    void simulateStep(float timeDelta);

    using IterateCirclesCallback = std::function<
      void(const Point& position, double radius, const Vector2d& movementDirection, float speed)>;

    void forEachCircle(const IterateCirclesCallback& callback);

    const Quadtree& getQuadtree() const;

private:
    Point m_bottomLeft;
    Point m_topRight;
    float m_radius;
    Quadtree m_quadtree;
    std::vector<CircleData> m_circles;
};
}

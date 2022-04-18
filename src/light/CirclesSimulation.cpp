#include "CirclesSimulation.h"

#include <random>

namespace
{
std::vector<light::Point> generatePoints(size_t count)
{
    std::random_device rd;
    std::mt19937 mt{ rd() };
    std::uniform_real_distribution<double> distX(0.2, 0.9);
    std::uniform_real_distribution<double> distY(0.4, 0.9);

    std::vector<light::Point> points;
    points.reserve(count);

    for (size_t i = 0; i < count; ++i)
    {
        const auto x = distX(mt);
        const auto y = distY(mt);
        points.emplace_back(x, y);
    }

    return points;
}

light::Vector2d reflect(const light::Vector2d& normal, const light::Vector2d& ray)
{
    return ray - 2 * glm::dot(normal, ray) * normal;
}

bool isCollided(const light::Point& center1,
                float radius1,
                const light::Point& center2,
                float radius2)
{
    const auto centerToCenterDistance = glm::length(center1 - center2);
    const auto minimumDistance = radius1 + radius2;
    return centerToCenterDistance < minimumDistance;
}

const light::Vector2d BoxLeftSideNormal{ 1, 0 };
const light::Vector2d BoxRightSideNormal{ -BoxLeftSideNormal };
const light::Vector2d BoxBottomSideNormal{ 0, 1 };
const light::Vector2d BoxTopSideNormal{ -BoxBottomSideNormal };

}

namespace light
{
CirclesSimulation::CirclesSimulation(const Point& bottomLeft,
                                     const Point& topRight,
                                     size_t circlesCount,
                                     float circleRadius,
                                     float speed)
  : m_bottomLeft{ bottomLeft }
  , m_topRight{ topRight }
  , m_radius{ circleRadius }
  , m_quadtree{ bottomLeft, topRight }
{
    std::random_device rd;
    std::mt19937 mt{ rd() };
    std::uniform_real_distribution<float> positionDist(2 * m_radius, 1.0f - 2 * m_radius);
    std::uniform_real_distribution<float> directionDist(-1, 1);

    const Point circleRectHalfSize{ m_radius, m_radius };

    m_circles.reserve(circlesCount);
    m_quadtree.reserve(circlesCount);
    for (size_t i = 0; i < circlesCount; ++i)
    {
        const auto posX = positionDist(mt);
        const auto posY = positionDist(mt);
        const Point circleCenter{ posX, posY };
        const auto dirX = directionDist(mt);
        const auto dirY = directionDist(mt);
        const auto direction = glm::normalize(Vector2d{ dirX, dirY });

        const auto circleBottomLeft = circleCenter - circleRectHalfSize;
        const auto circleTopRight = circleCenter + circleRectHalfSize;

        bool isOverlaps = false;

        m_quadtree.forEachObjectInArea(circleBottomLeft,
                                       circleTopRight,
                                       [&](const Id& id, const auto _, const auto)
                                       { const auto& existingCircle = m_circles[id]; });

        if (isOverlaps)
        {
            --i;
            continue;
        }

        m_circles.push_back(CircleData{ speed, circleCenter, direction });
        m_quadtree.insert(circleBottomLeft, circleTopRight, Id(i));
    }
}

size_t CirclesSimulation::size() const
{
    return m_circles.size();
}

void CirclesSimulation::simulateStep(float timeDelta)
{
    const Point circleRectHalfSize{ m_radius, m_radius };

    // update circles position
    for (size_t i = 0; i < size(); ++i)
    {
        auto& currentCircle = m_circles[i];
        currentCircle.position += currentCircle.movementDirection * timeDelta * currentCircle.speed;
    }

    // check for collision and update movement direction
    for (size_t i = 0; i < size(); ++i)
    {
        auto& circle1 = m_circles[i];
        m_quadtree.forEachObjectInArea(
          circle1.position - circleRectHalfSize,
          circle1.position + circleRectHalfSize,
          [&](const Id& id, const auto, const auto)
          {
              if (i == id)
              {
                  return;
              }

              auto& circle2 = m_circles[id];

              // if collision is considered - resolve it

              // circles are considered as collided
              if (isCollided(circle1.position, m_radius, circle2.position, m_radius))
              {
                  // todo add masses/speed handling

                  const Vector2d normal{ glm::normalize(circle1.position - circle2.position) };

                  if (glm::dot(normal, circle1.movementDirection) < 0 ||
                      glm::dot(-normal, circle2.movementDirection) < 0)
                  {
                      const auto reflectedDir1 = reflect(normal, circle1.movementDirection);
                      const auto reflectedDir2 = reflect(-normal, circle2.movementDirection);
                      circle1.movementDirection = glm::normalize(reflectedDir1);
                      circle2.movementDirection = glm::normalize(reflectedDir2);
                  }
              }
          });

        // box sides collision handling

        // left side
        if (circle1.position.x - m_radius < m_bottomLeft.x)
        {
            circle1.position.x = m_bottomLeft.x + m_radius;
            circle1.movementDirection = reflect(BoxLeftSideNormal, circle1.movementDirection);
        }

        // bottom side
        if (circle1.position.y - m_radius < m_bottomLeft.y)
        {
            circle1.position.y = m_bottomLeft.y + m_radius;
            circle1.movementDirection = reflect(BoxBottomSideNormal, circle1.movementDirection);
        }

        // right side
        /* && glm::dot(circle1.movementDirection, BoxRightSideNormal) < 0 */
        if (circle1.position.x + m_radius > m_topRight.x)
        {
            circle1.position.x = m_topRight.x - m_radius;
            circle1.movementDirection = reflect(BoxRightSideNormal, circle1.movementDirection);
        }

        // top side
        if (circle1.position.y + m_radius > m_topRight.y)
        {
            circle1.position.y = m_topRight.y - m_radius;
            circle1.movementDirection = reflect(BoxTopSideNormal, circle1.movementDirection);
        }
    }
}

void CirclesSimulation::forEachCircle(const IterateCirclesCallback& callback)
{
    for (const auto& circle : m_circles)
    {
        callback(circle.position, m_radius, circle.movementDirection, circle.speed);
    }
}

const Quadtree& CirclesSimulation::getQuadtree() const
{
    return m_quadtree;
}

}

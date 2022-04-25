#include "CirclesSimulation.h"

#include <random>
#include <stdexcept>

namespace
{

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

std::tuple<light::Point, light::Point> getCircleCorners(const light::Point& center, float radius)
{
    const light::Point circleRectHalfSize{ radius, radius };
    const auto circleBottomLeft = center - circleRectHalfSize;
    const auto circleTopRight = center + circleRectHalfSize;
    return { circleBottomLeft, circleTopRight };
}

const light::Vector2d BoxLeftSideNormal{ 1, 0 };
const light::Vector2d BoxRightSideNormal{ -BoxLeftSideNormal };
const light::Vector2d BoxBottomSideNormal{ 0, 1 };
const light::Vector2d BoxTopSideNormal{ -BoxBottomSideNormal };

const auto MAX_INSERT_TRIES = 1000;

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

    m_circles.reserve(circlesCount);
    m_quadtree.reserve(circlesCount);

    for (size_t i = 0; i < circlesCount; ++i)
    {
        const auto dirX = directionDist(mt);
        const auto dirY = directionDist(mt);
        const auto direction = glm::normalize(Vector2d{ dirX, dirY });

        size_t triesCount = 0;
        bool isOverlaps = true;

        while (triesCount++ < MAX_INSERT_TRIES && isOverlaps)
        {
            const auto posX = positionDist(mt);
            const auto posY = positionDist(mt);
            const Point circleCenter{ posX, posY };

            isOverlaps = false;

            const auto [circleBottomLeft, circleTopRight] =
              getCircleCorners(circleCenter, m_radius);

            m_quadtree.forEachObjectInArea(
              circleBottomLeft,
              circleTopRight,
              [&](const Id& id, const auto _, const auto)
              {
                  const auto& existingCircle = m_circles[id];
                  if (isCollided(circleCenter, m_radius, existingCircle.position, m_radius))
                  {
                      isOverlaps = true;
                      return false;
                  }
                  return true;
              });

            if (!isOverlaps)
            {
                m_circles.push_back(CircleData{ speed, circleCenter, direction });
                m_quadtree.insert(circleBottomLeft, circleTopRight, Id(i));
            }
        }

        if (triesCount >= MAX_INSERT_TRIES)
        {
            throw std::runtime_error("Failed to insert point. The points number is too big or "
                                     "point radius is too big.");
        }
    }

    /*m_circles.push_back(CircleData{ speed, { 0.2, 0.5 }, { 1, 0 } });
    m_circles.push_back(CircleData{ speed, { 0.6, 0.5 }, { -1, 0 } });    */

    m_quadtree.clear();
}

size_t CirclesSimulation::size() const
{
    return m_circles.size();
}

void CirclesSimulation::simulateStep(float timeDelta)
{
    // update circles position and build quadtree
    const Point circleRectHalfSize{ m_radius, m_radius };

    m_quadtree.clear();
    assert(m_quadtree.size() == 0);

    for (size_t i = 0; i < size(); ++i)
    {
        auto& currentCircle = m_circles[i];
        currentCircle.position += currentCircle.movementDirection * timeDelta * currentCircle.speed;

        const auto [circleBottomLeft, circleTopRight] =
          getCircleCorners(currentCircle.position, m_radius);
        m_quadtree.insert(circleBottomLeft, circleTopRight, Id(i));
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
                  return true;
              }

              auto& circle2 = m_circles[id];

              // if collision is considered - resolve it

              // circles are considered as collided
              if (isCollided(circle1.position, m_radius, circle2.position, m_radius))
              {
                  // todo add masses/speed handling

                  const Vector2d normal{ glm::normalize(circle2.position - circle1.position) };
                  const Vector2d firstToSecond{ glm::normalize(circle2.position -
                                                               circle1.position) };

                  const auto cosAngle1 = glm::dot(firstToSecond, circle1.movementDirection);
                  if (cosAngle1 > 0)
                  {
                      const auto reflectedDir1 = reflect(normal, circle1.movementDirection);
                      circle1.movementDirection = glm::normalize(reflectedDir1);
                  }

                  const auto cosAngle2 = glm::dot(-firstToSecond, circle2.movementDirection);
                  if (cosAngle2 > 0)
                  {
                      const auto reflectedDir2 = reflect(normal, circle2.movementDirection);
                      circle2.movementDirection = glm::normalize(reflectedDir2);
                  }
              }

              return true;
          });

        // box sides collision handling:

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

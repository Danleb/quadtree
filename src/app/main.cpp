#include <quadtree/Quadtree.h>

#include <SFML/Graphics.hpp>

#include <random>
#include <vector>

constexpr auto CIRCLES_COUNT = 170;

std::vector<light::Point> generatePoints()
{
    std::random_device rd;
    // std::mt19937 mt{};
    std::mt19937 mt{ rd() };
    std::uniform_real_distribution<double> distX(0.3, 0.9);
    std::uniform_real_distribution<double> distY(0.1, 0.9);

    std::vector<light::Point> points;
    points.reserve(CIRCLES_COUNT);

    for (size_t i = 0; i < CIRCLES_COUNT; ++i)
    {
        const auto x = distX(mt);
        const auto y = distY(mt);
        points.emplace_back(x, y);
    }

    return points;
}

int main()
{
    light::Point bottomLeft(0, 0);
    light::Point topRight(1, 1);
    light::Quadtree quadtree(bottomLeft, topRight);

    const auto points = generatePoints();

    const auto circleRadius = 0.005;
    const light::Point halfSize(circleRadius, circleRadius);

    for (size_t i = 0; i < points.size(); ++i)
    {
        const auto p = points[i];
        const auto minXY = p - halfSize;
        const auto maxXY = p + halfSize;
        quadtree.insert(minXY, maxXY, 0);
    }

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Quadtree visualization");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window.close();
                    break;
                }
                case sf::Event::Resized:
                {
                    window.setView(
                      sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
                    break;
                }
                default:
                    break;
            }
        }

        window.clear();

        const auto windowSize = window.getSize();

        // clang-format off
        glm::mat3 scale{
            windowSize.x, 0, 0,
            0, -static_cast<float>(windowSize.y), 0,
            0, 0, 1
        };
        glm::mat3 position{
            1, 0, 0,
            0, 1, 0,
            0, windowSize.y, 1
        };
        // clang-format on
        glm::mat3 toScreenSpace = position * scale;
        using Point3 = glm::vec3;

        for (size_t i = 0; i < points.size(); ++i)
        {
            const auto p = points[i];
            const auto windowPoint = toScreenSpace * Point3(p.x, p.y, 1);
            const auto circleRadiusPx = windowSize.x * circleRadius;
            sf::CircleShape circle(circleRadiusPx, circleRadiusPx);
            circle.setFillColor(sf::Color::Green);
            circle.setPosition(windowPoint.x - circleRadiusPx, windowPoint.y - circleRadiusPx);
            window.draw(circle);
        }

        const int quadBorderThickness = 5;
        const auto halfThickness = quadBorderThickness / 2;
        sf::RectangleShape horizontalLine(
          { static_cast<float>(windowSize.x), quadBorderThickness });
        horizontalLine.setFillColor(sf::Color::Red);
        sf::RectangleShape verticalLine({ quadBorderThickness, static_cast<float>(windowSize.y) });
        verticalLine.setFillColor(sf::Color::Red);

        quadtree.traverseQuads(
          [&](light::Point bottomLeft, light::Point size)
          {
              horizontalLine.setSize(
                sf::Vector2f{ size.x * windowSize.x, static_cast<float>(quadBorderThickness) });
              verticalLine.setSize(
                sf::Vector2f{ static_cast<float>(quadBorderThickness), size.y * windowSize.y });

              auto bottomLeft3d = Point3(bottomLeft, 1);
              const auto corner1 = toScreenSpace * bottomLeft3d;
              horizontalLine.setPosition(corner1.x, corner1.y - halfThickness);
              window.draw(horizontalLine);

              bottomLeft3d.y += size.y;
              const auto corner2 = toScreenSpace * bottomLeft3d;
              horizontalLine.setPosition(corner2.x, corner2.y - halfThickness);
              window.draw(horizontalLine);

              verticalLine.setPosition(corner2.x - halfThickness, corner2.y);
              window.draw(verticalLine);

              bottomLeft3d.x += size.x;
              const auto corner3 = toScreenSpace * bottomLeft3d;
              verticalLine.setPosition(corner3.x - halfThickness, corner3.y);
              window.draw(verticalLine);
          });

        window.display();
    }
    return EXIT_SUCCESS;
}

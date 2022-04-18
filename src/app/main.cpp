#include <light/CirclesSimulation.h>
#include <light/Quadtree.h>

#include <SFML/Graphics.hpp>

int main()
{
    light::Point bottomLeft(0, 0);
    light::Point topRight(1, 1);
    const auto circleRadius = 0.005;
    const auto circlesCount = 300;
    const auto speed = 0.3;
    light::CirclesSimulation simulation{ bottomLeft, topRight, circlesCount, circleRadius, speed };

    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Quadtree visualization");
    window.setFramerateLimit(60);
    sf::Clock clock;

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

        const auto elapsedTime = clock.getElapsedTime().asSeconds();
        clock.restart();
        // simulation.simulateStep(1 / 60.0);
        // simulation.simulateStep(clock.getElapsedTime().asSeconds());

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

        // Draw circles
        simulation.forEachCircle(
          [&](const light::Point& position,
              const auto radius,
              const light::Vector2d& movementDirection,
              float speed)
          {
              const auto windowPoint = toScreenSpace * Point3(position.x, position.y, 1);
              const auto circleRadiusPx = windowSize.x * radius;
              sf::CircleShape circle(circleRadiusPx, circleRadiusPx);
              circle.setFillColor(sf::Color::Green);
              circle.setPosition(windowPoint.x - circleRadiusPx, windowPoint.y - circleRadiusPx);

              /*auto d = movementDirection;
              d.y *= -1;
              auto x = windowPoint + Point3(d * 300.0f, 1);
              sf::Vertex line[] = { sf::Vertex(sf::Vector2f(windowPoint.x, windowPoint.y)),
                                    sf::Vertex(sf::Vector2f(x.x, x.y)) };
              window.draw(line, 2, sf::Lines);*/

              window.draw(circle);
          });

        // Draw quads
        const int quadBorderThickness = 5;
        const auto halfThickness = quadBorderThickness / 2;
        sf::RectangleShape horizontalLine(
          { static_cast<float>(windowSize.x), quadBorderThickness });
        horizontalLine.setFillColor(sf::Color::Red);
        sf::RectangleShape verticalLine({ quadBorderThickness, static_cast<float>(windowSize.y) });
        verticalLine.setFillColor(sf::Color::Red);

        simulation.getQuadtree().traverseQuads(
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

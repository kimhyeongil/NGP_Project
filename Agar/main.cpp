#include <cmath>
#include <iostream>
#include "Common.h"
#include "Entity.h"

using namespace std;

int main()
{

    const int windowWidth = 800;
    const int windowHeight = 600;
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "SFML Moving Circle");
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(0, 0, windowWidth, windowHeight));

    Entity entity;

    sf::CircleShape player(20.f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(player.getRadius(), player.getRadius());

    sf::Vector2f playerPos(windowWidth / 2.f, windowHeight / 2.f);

    view.setCenter(playerPos);

    sf::Clock clock;

    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        sf::Event event;
        std::vector<sf::Event> events;

        while (window.pollEvent(event))
        {
            events.emplace_back(event);
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2i mousePixel(event.mouseButton.x, event.mouseButton.y);
                    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel, view);

                    cout << sf::Vector2f::Distance(mouseWorld, playerPos) << endl;
                    cout << "클릭 월드 위치: (" << mouseWorld.x << ", " << mouseWorld.y << ")" << endl;
                }
            }
        }
        entity.HandleInput(events);
        window.clear(sf::Color::White);

        window.setView(view);

        entity.Render(window);

        window.display();
    }

    return 0;
}

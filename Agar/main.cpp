#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <iostream>

using namespace std;

int main()
{

    const int windowWidth = 800;
    const int windowHeight = 600;
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "SFML Moving Circle");
    window.setFramerateLimit(60);

    sf::View view(sf::FloatRect(0, 0, windowWidth, windowHeight));


    sf::CircleShape player(20.f);
    player.setFillColor(sf::Color::Green);
    player.setOrigin(player.getRadius(), player.getRadius());

    sf::Vector2f playerPos(windowWidth / 2.f, windowHeight / 2.f);
    player.setPosition(playerPos);

    view.setCenter(playerPos);

    sf::Clock clock;

    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2i mousePixel(event.mouseButton.x, event.mouseButton.y);
                    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel, view);

                    cout << "클릭 월드 위치: (" << mouseWorld.x << ", " << mouseWorld.y << ")" << endl;
                }
            }
        }

        window.clear(sf::Color::White);

        window.setView(view);

        window.draw(player);

        window.display();
    }

    return 0;
}

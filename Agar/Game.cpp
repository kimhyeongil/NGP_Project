#include "Game.h"

using namespace std;
using namespace sf;

Game::Game()
    : window(VideoMode(windowWidth, windowHeight), "NGP")
    , scene{make_unique<PlayScene>()}
{
    window.setFramerateLimit(60);
}

Game& Game::Instance()
{
    static Game game;
    return game;
}

sf::Vector2f Game::WorldMouse(const sf::Vector2i& screenMouse)
{
    return window.mapPixelToCoords(screenMouse, scene->view);
}

void Game::Run()
{
    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Time deltaTime = clock.restart();
        ProcessEvents();
        Update(deltaTime);
        Render();
    }
}

void Game::ProcessEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
        else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
            window.close();
        else
            scene->HandleEvent(event);
    }
}


void Game::Update(sf::Time deltaTime)
{
    scene->Update(deltaTime);
}

void Game::Render()
{
    window.clear(sf::Color::White);

    scene->Render(window);

    window.display();
}

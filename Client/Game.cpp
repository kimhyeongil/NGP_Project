#include <thread>
#include <iostream>
#include "Game.h"

using namespace std;
using namespace sf;

Game::Game()
    : window(VideoMode(windowWidth, windowHeight), "NGP")
    , scene{make_unique<PlayScene>()}
    , networkManger{ make_unique<ClientNetworkManager>() }
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

void Game::Send(PACKET packet)
{
    networkManger->AddPacket(packet);
}

void Game::Run()
{
    string name;
    cout << "이름 입력: ";
    cin >> name;

    // 이름 패킷 전송
    PACKET namePacket;
    namePacket.type = PACKET_TYPE::PLAYER_NAME;
    strncpy(namePacket.data.name, name.c_str(), sizeof(namePacket.data.name) - 1);
    namePacket.data.name[sizeof(namePacket.data.name) - 1] = '\0'; // NULL-terminate
    Send(namePacket);

    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Time deltaTime = clock.restart();
        ProcessInputs();
        Update(deltaTime);
        Render();
    }
}

void Game::ProcessInputs()
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

    auto recvPacket = networkManger->GetPacket();
    for (auto& packet : recvPacket) {
        scene->HandlePacket(packet);
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

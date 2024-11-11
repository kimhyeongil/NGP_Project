#include <thread>
#include <iostream>
#include "Game.h"

using namespace std;
using namespace sf;

Game::Game()
    : window(VideoMode(windowWidth, windowHeight), "NGP")
    , scene{make_unique<PlayScene>()}
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        exit(-1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        exit(-1);
    }
    window.setFramerateLimit(60);
}

Game::~Game()
{
    WSACleanup();
    closesocket(sock);
}

Game& Game::Instance()
{
    static Game game;
    return game;
}

bool Game::Init(char* serverIP, short serverPORT)
{
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(serverIP);
    serveraddr.sin_port = htons(serverPORT);

    if (connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
        return false;
    }

    return true;
}

sf::Vector2f Game::WorldMouse(const sf::Vector2i& screenMouse)
{
    return window.mapPixelToCoords(screenMouse, scene->view);
}

void Game::Send(const PACKET& packet)
{
    if (packet.type == PACKET_TYPE::PLAYER_INPUT) {
        uint type = htonl(packet.type);
        PlayerInput copy = *(PlayerInput*)packet.context;
        cout << copy.x << ", " << copy.y << endl;
        copy.hton();


        thread([sock = this->sock, copy, type] (){
            send(sock, (char*)&type, sizeof(uint), 0);
            cout << copy.x << ", " << copy.y << endl;
            send(sock, (char*)&copy, sizeof(PlayerInput), 0);
            }).detach();
    }

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

#pragma once
#include "../Common.h"
#include "GameCommon.h"
#include "Scene.h"
#include <memory>

class Game {
    Game();
    ~Game();
public:
    static Game& Instance();

    bool Init(char* serverIP, short serverPORT);
    void Run();

    void Send(const PACKET& packet);

    static constexpr int windowWidth = 800;
    static constexpr int windowHeight = 600;
    
    sf::Vector2f WorldMouse(const sf::Vector2i& screenMouse);

    sf::RenderWindow window;
private:
    void ProcessEvents();
    void Update(sf::Time deltaTime);
    void Render();

    SOCKET sock;
    std::unique_ptr<Scene> scene;
};




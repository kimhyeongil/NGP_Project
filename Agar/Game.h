#pragma once
#include "Common.h"
#include "Scene.h"
#include <memory>

class Game {
    Game();
public:
    static Game& Instance();

    void Run();

    static constexpr int windowWidth = 800;
    static constexpr int windowHeight = 600;
    
    sf::Vector2f WorldMouse(const sf::Vector2i& screenMouse);

    sf::RenderWindow window;
private:
    void ProcessEvents();
    void Update(sf::Time deltaTime);
    void Render();


    std::unique_ptr<Scene> scene;
};




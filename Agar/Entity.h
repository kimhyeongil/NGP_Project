#pragma once
#include <vector>
#include "Common.h"

class Entity
{
public:
	Entity();

	void HandleInput(const std::vector<sf::Event>&);
	void Render(sf::RenderWindow&);
private:
	sf::CircleShape shape;
	sf::Vector2f position;
};


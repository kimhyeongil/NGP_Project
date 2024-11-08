#pragma once
#include <vector>
#include "Common.h"

class Entity : public sf::Drawable {
public:
	Entity();

	virtual void HandleInput(const sf::Event& event) {}
	virtual void Update(double deltaTime) {}

	sf::Vector2f Position() const { return shape.getPosition(); }
protected:
	virtual void draw(sf::RenderTarget&, sf::RenderStates) const;

	sf::CircleShape shape;
	unsigned int color = 0;
};

class Player : public Entity
{
public:
	Player();

	virtual void HandleInput(const sf::Event& event);
	virtual void Update(double deltaTime);
private:
	sf::Vector2f destination;
	float speed = 200;
	float size;
};

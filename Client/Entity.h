#pragma once
#include <vector>
#include "Common.h"
#include "GameCommon.h"

class Entity : public sf::Drawable {
public:
	Entity();

	virtual void Update(double deltaTime) {}

	sf::Vector2f Position() const { return shape.getPosition(); }

	virtual void draw(sf::RenderTarget&, sf::RenderStates) const;

	sf::CircleShape shape;
	unsigned int color = 0;

	int id;
};

class Player : public Entity {
public:
	static constexpr float startSize = 20.f;
	Player();
	Player(const LoginSuccess::PlayerInfo&);
	Player(const PlayerAppend&);

	void Update(double deltaTime) override;

	void SetDestination(const sf::Vector2f&);
	void SetDestination(float x, float y);

	void SetPosition(const sf::Vector2f&);

	char name[16];
	sf::Vector2f destination;
	float speed = 200;
	float size;
};

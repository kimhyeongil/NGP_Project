#pragma once
#include <vector>
#include "Common.h"
#include "GameCommon.h"

class Entity : public sf::Drawable {
public:
	Entity(int id) :id{ id } {}

	virtual void Update(double deltaTime) {}
	virtual void OnCollision(const Entity* collider) {}

	sf::Vector2f Position() const { return shape.getPosition(); }
	float Radius() const { return shape.getRadius(); }

	virtual void SetPosition(const sf::Vector2f& pos) { shape.setPosition(pos); }
	virtual void SetPosition(float x, float y) { shape.setPosition(sf::Vector2f{ x, y }); }

	virtual void draw(sf::RenderTarget&, sf::RenderStates) const;

	int id;
	bool active = true;
protected:
	sf::CircleShape shape;
};

class Player : public Entity {
public:
	static constexpr int startSize = 200;

	Player(int id);
	Player(const PlayerInfo&);
	Player(const PlayerAppend&);

	void Update(double deltaTime) override;
	void OnCollision(const Entity* collider) override;

	void SetDestination(const sf::Vector2f&);
	void SetDestination(float x, float y);
	const sf::Vector2f& Destination() const { return destination; }

	void SetPosition(const sf::Vector2f&) override;
	void SetPosition(float x, float y) override { SetPosition(sf::Vector2f{ x,y }); };

	void SetColor(uint newColor);

	void draw(sf::RenderTarget&, sf::RenderStates) const override;

	char name[16];
	float speed = 200;
	int size{ startSize };
	uint color = 0;
private:
	sf::Vector2f destination;

};

class Food : public Entity{
	static constexpr float maxTime = 5;
public:
	static constexpr float defaultSize = 5;

	Food(int id);
	Food(const FoodInfo&);

	void Reset();
	void Update(double deltaTime) override;
	void OnCollision(const Entity* collider) override;

	float activeTime = 0;
};

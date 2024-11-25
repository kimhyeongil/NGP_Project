#pragma once
#include <vector>
#include "Common.h"
#include "GameCommon.h"

class Entity : public sf::Drawable {
public:
	Entity();

	bool Active() const { return active; }
	void SetActive(bool newActive);
	virtual void OnActive() {}
	virtual void OnInactive() {}

	virtual void Update(double deltaTime) {}

	sf::Vector2f Position() const { return shape.getPosition(); }
	virtual void SetPosition(const sf::Vector2f& pos) { shape.setPosition(pos); }
	virtual void SetPosition(float x, float y) { shape.setPosition(sf::Vector2f{ x, y }); }

	virtual void draw(sf::RenderTarget&, sf::RenderStates) const;

	unsigned int color = 0;

	int id;

protected:
	sf::CircleShape shape;
private:
	bool active = true;
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
	const sf::Vector2f& Destination() const { return destination; }

	void SetPosition(const sf::Vector2f&) override;

	void draw(sf::RenderTarget&, sf::RenderStates) const override;

	char name[16];
	float speed = 200;
	float size;
private:
	sf::Vector2f destination;

};

class Food : public Entity{
	static constexpr float maxTime = 5;
public:
	void OnActive() override;

	void Update(double deltaTime) override;

private:
	float activeTime = 0;
};

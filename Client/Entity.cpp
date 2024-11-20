#include "Entity.h"
#include "Game.h"
#include "Scene.h"
#include "Random.h"
#include <iostream>

using namespace std;
using namespace sf;

Entity::Entity()
	: shape{ 20.f }
{
	shape.setFillColor(colors[color]);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(shape, states);
}

Player::Player()
	: Entity{}
	, size{ startSize }
{
	shape.setRadius(size);
	shape.setPosition(-1, -1);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

Player::Player(const LoginSuccess::PlayerInfo& info)
{
	memcpy(name, info.name, 16);
	size = info.size;
	color = info.color;
	id = info.id;
	shape.setPosition(info.x, info.y);
	shape.setRadius(size);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
	shape.setFillColor(colors[color]);
	destination = Position();
}

Player::Player(const PlayerAppend& info)
	: size{startSize}
{
	color = info.color;
	id = info.id;
	shape.setPosition(info.x, info.y);
	shape.setRadius(size);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
	shape.setFillColor(colors[color]);
	destination = Position();
}


void Player::SetDestination(const sf::Vector2f& dest)
{
	destination = dest;
}

void Player::SetPosition(const sf::Vector2f& pos)
{
	destination = pos;
	shape.setPosition(pos);
}

void Player::Update(double deltaTime)
{
	auto position = shape.getPosition();
	if (Vector2f::Distance(position, destination) > 0.0001) {
		auto dir = Vector2f::Normalize(destination - position);
		auto distance = min(deltaTime * speed, Vector2f::Distance(position, destination));
		shape.move(dir * distance);

		position = shape.getPosition();
		position = Vector2f::Min(position, Vector2f{ PlayScene::worldWidth - size, PlayScene::worldHeight - size });
		position = Vector2f::Max(position, Vector2f{ size, size });

		shape.setPosition(position);
	}
	
}
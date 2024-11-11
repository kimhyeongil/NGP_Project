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
	, size{ shape.getRadius() }
{
	shape.setPosition(Random::RandInt(size, PlayScene::worldWidth - size), Random::RandInt(size, PlayScene::worldHeight - size));
	destination = shape.getPosition();
}

void Player::HandleInput(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed &&
		event.mouseButton.button == sf::Mouse::Left) {
		destination = Game::Instance().WorldMouse(Vector2i{ event.mouseButton.x, event.mouseButton.y });
		auto dir = destination - Position();
		PlayerInput input(dir.x, dir.y);
		PACKET packet;
		packet.type = PLAYER_INPUT;
		packet.context = &input;
		Game::Instance().Send(packet);
	}
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
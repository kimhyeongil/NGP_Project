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
	memcpy(name, info.name, 16);
	shape.setPosition(info.x, info.y);
	shape.setRadius(size);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
	shape.setFillColor(colors[color]);
	destination = Position();
}


void Player::SetDestination(const sf::Vector2f& dest)
{
	
	destination = Vector2f::Min(dest, Vector2f{ PlayScene::worldWidth - size, PlayScene::worldHeight - size });
	destination = Vector2f::Max(destination, Vector2f{ size, size });
}

void  Player::SetDestination(float x, float y)
{
	SetDestination(sf::Vector2f(x, y));
}

void Player::SetPosition(const sf::Vector2f& pos)
{
	auto position = pos;
	position = Vector2f::Min(position, Vector2f{ PlayScene::worldWidth - size, PlayScene::worldHeight - size });
	position = Vector2f::Max(position, Vector2f{ size, size });
	shape.setPosition(position);
	destination = position;

}

void Player::Update(double deltaTime)
{
	auto position = shape.getPosition();
	if (Vector2f::Distance(position, destination) > 1e-5) {
		auto dir = Vector2f::Normalize(destination - position);
		auto distance = min(deltaTime * speed, Vector2f::Distance(position, destination));
		shape.move(dir * distance);

		position = Position();
		position = Vector2f::Min(position, Vector2f{ PlayScene::worldWidth - size, PlayScene::worldHeight - size });
		position = Vector2f::Max(position, Vector2f{ size, size });

		shape.setPosition(position);
	}
	
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(shape, states);

	// 텍스트를 그릴 준비
	static sf::Font font;
	static bool isFontLoaded = font.loadFromFile("consola.ttf");
	if (!isFontLoaded) {
		return;
	}

	// 텍스트 설정
	sf::Text text;
	text.setFont(font);
	text.setString(name);
	text.setCharacterSize(size / 2);
	text.setFillColor(sf::Color::Black);

	// 텍스트의 중심을 설정
	sf::FloatRect textBounds = text.getLocalBounds();
	text.setOrigin(textBounds.worldWidth / 2.f, textBounds.worldHeight / 2.f);

	// 텍스트를 플레이어의 중앙에 배치
	sf::Vector2f playerCenter = Position();
	text.setPosition(playerCenter);

	// 텍스트를 그린다
	target.draw(text, states);
}

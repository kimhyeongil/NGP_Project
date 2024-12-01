#include "Entity.h"
#include "Game.h"
#include "Scene.h"
#include "Random.h"
#include <iostream>

using namespace std;
using namespace sf;

PlayerInfo::PlayerInfo(const Player& player)
{
	id = player.id;
	color = player.color;
	size = player.size;
	x = player.Position().x;
	y = player.Position().y;
	memcpy(name, player.name, 16);
}

FoodInfo::FoodInfo(const Food& food)
{
	id = food.id;
	active = food.active;
	activeTime = food.activeTime;
	x = food.Position().x;
	y = food.Position().y;
}

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (active) {
		target.draw(shape, states);
	}
}

Player::Player(int id)
	: Entity{ id }
{
	shape.setFillColor(colors[color]);
	shape.setRadius(size / 10.f);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

Player::Player(const PlayerInfo& info)
	:Entity{ info.id }
{
	memcpy(name, info.name, 16);
	size = info.size;
	color = info.color;
	shape.setRadius(size / 10.f);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
	shape.setFillColor(colors[color]);
	shape.setPosition(info.x, info.y);
	destination = Position();
}

Player::Player(const PlayerAppend& info)
	:Entity{ info.id }
{
	color = info.color;
	memcpy(name, info.name, 16);
	shape.setRadius(size / 10.f);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
	shape.setFillColor(colors[color]);
	shape.setPosition(info.x, info.y);
	destination = Position();
}

void Player::SetDestination(const sf::Vector2f& dest)
{
	destination = Vector2f::Min(dest, Vector2f{ PlayScene::worldWidth - shape.getRadius(), PlayScene::worldHeight - shape.getRadius() });
	destination = Vector2f::Max(destination, Vector2f{ shape.getRadius(), shape.getRadius() });
}

void  Player::SetDestination(float x, float y)
{
	SetDestination(sf::Vector2f(x, y));
}

void Player::SetPosition(const sf::Vector2f& pos)
{
	auto position = pos;
	position = Vector2f::Min(position, Vector2f{ PlayScene::worldWidth - shape.getRadius(), PlayScene::worldHeight - shape.getRadius() });
	position = Vector2f::Max(position, Vector2f{ shape.getRadius(), shape.getRadius() });
	shape.setPosition(position);
	destination = position;

}

void Player::SetColor(uint newColor)
{
	if (color != newColor) {
		color = newColor;
		shape.setFillColor(colors[color]);
	}
}

void Player::Update(double deltaTime)
{
	if (active) {
		auto position = shape.getPosition();
		if (Vector2f::Distance(position, destination) > 1e-5) {
			auto dir = Vector2f::Normalize(destination - position);
			auto distance = min(deltaTime * speed, Vector2f::Distance(position, destination));
			shape.move(dir * distance);

			position = Position();
			position = Vector2f::Min(position, Vector2f{ PlayScene::worldWidth - shape.getRadius(), PlayScene::worldHeight - shape.getRadius() });
			position = Vector2f::Max(position, Vector2f{ shape.getRadius(), shape.getRadius() });

			shape.setPosition(position);
		}
	}
}

void Player::OnCollision(const Entity* collider) {
	if (auto other = dynamic_cast<const Player*>(collider); other != nullptr) {
		if (other->size > size) {
			active = false;
		}
		else if (other->size < size) {
			size += other->size / 2;
		}
	}
	else if (auto other = dynamic_cast<const Food*>(collider); other != nullptr) {
		size += Food::defaultSize;
	}
	shape.setRadius(size / 10.f);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (active) {
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
		text.setCharacterSize(shape.getRadius() / 2);
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
}

Food::Food(int id)
	:Entity{ id }
{
	shape.setRadius(defaultSize);
	shape.setFillColor(Random::RandColor());
	shape.setOrigin(shape.getRadius(), shape.getRadius());

	Reset();
}

Food::Food(const FoodInfo& info)
	:Entity{ info.id }
{
	activeTime = info.activeTime;
	active = info.active;
	SetPosition(info.x, info.y);
	shape.setRadius(defaultSize);
	shape.setFillColor(Random::RandColor());
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

void Food::OnCollision(const Entity* collider)
{
	if (auto other = dynamic_cast<const Player*>(collider); other != nullptr) {
		active = false;
	}
}

void Food::Update(double deltaTime)
{
	if (active) {
		activeTime += deltaTime;
		auto color = shape.getFillColor();
		color.a = 255 * (maxTime - activeTime) / maxTime;
		shape.setFillColor(color);
		if (activeTime > maxTime) {
			active = false;
		}
	}
}

void Food::Reset()
{
	active = true;
	activeTime = 0;
	SetPosition(Random::RandInt(Food::defaultSize, PlayScene::worldWidth - Food::defaultSize), 
				Random::RandInt(Food::defaultSize, PlayScene::worldHeight - Food::defaultSize));
}
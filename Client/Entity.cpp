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
	activeTime = food.activeTime;
	x = food.Position().x;
	y = food.Position().y;
}

void Entity::SetActive(bool newActive)
{
	if (newActive == active) {
		return;
	}
	active = newActive;
	if (active) {
		OnActive();
	}
	else {
		OnInactive();
	}
}

void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (Active()) {
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
{
	memcpy(name, info.name, 16);
	size = info.size;
	color = info.color;
	id = info.id;
	shape.setRadius(size / 10.f);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
	shape.setFillColor(colors[color]);
	shape.setPosition(info.x, info.y);
	destination = Position();
}

Player::Player(const PlayerAppend& info)
{
	color = info.color;
	id = info.id;
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

void Player::Update(double deltaTime)
{
	if (Active()) {
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
			SetActive(false);
		}
		else if (other->size < size) {
			size += other->size / 2;
		}
	}
	else if (auto other = dynamic_cast<const Food*>(collider); other != nullptr) {
		size += Food::defaultSize / 2;
	}
}

void Player::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (Active()) {
		target.draw(shape, states);

		// �ؽ�Ʈ�� �׸� �غ�
		static sf::Font font;
		static bool isFontLoaded = font.loadFromFile("consola.ttf");
		if (!isFontLoaded) {
			return;
		}

		// �ؽ�Ʈ ����
		sf::Text text;
		text.setFont(font);
		text.setString(name);
		text.setCharacterSize(shape.getRadius() / 2);
		text.setFillColor(sf::Color::Black);

		// �ؽ�Ʈ�� �߽��� ����
		sf::FloatRect textBounds = text.getLocalBounds();
		text.setOrigin(textBounds.worldWidth / 2.f, textBounds.worldHeight / 2.f);

		// �ؽ�Ʈ�� �÷��̾��� �߾ӿ� ��ġ
		sf::Vector2f playerCenter = Position();
		text.setPosition(playerCenter);

		// �ؽ�Ʈ�� �׸���
		target.draw(text, states);
	}
}

Food::Food()
{
	shape.setRadius(defaultSize);
	shape.setFillColor(Random::RandColor());
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

Food::Food(const FoodInfo& info)
{
	id = info.id;
	activeTime = info.activeTime;
	SetPosition(info.x, info.y);
	shape.setRadius(defaultSize);
	shape.setFillColor(Random::RandColor());
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

void Food::OnActive()
{
	shape.setFillColor(Random::RandColor());
	activeTime = 0;
}

void Food::Update(double deltaTime)
{
	if (Active()) {
		activeTime += deltaTime;
		auto color = shape.getFillColor();
		color.a = 255 * (maxTime - activeTime) / maxTime;
		shape.setFillColor(color);
		if (activeTime > maxTime) {
			//SetActive(false);
		}
	}
}
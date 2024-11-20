#include "Scene.h"
#include "Entity.h"
#include "Game.h"
#include <iostream>
using namespace std;
using namespace sf;

Scene::Scene()
	:view(FloatRect(0, 0, Game::windowWidth, Game::windowHeight))
{
	view.setSize(Game::windowWidth, Game::windowHeight);
}

PlayScene::PlayScene()
	:player{ make_unique<Player>() }
{
	view.setCenter(player->Position());

	Color lineColor{ 128,128,128 };
	float lineThickness = 2.5;
	for (int x = 0; x < worldWidth; x += space) {
		sf::RectangleShape line(sf::Vector2f(lineThickness, worldHeight));
		line.setPosition(x - lineThickness / 2.0f, 0);
		line.setFillColor(lineColor);
		world.push_back(line);
	}

	for (int y = 0; y < worldHeight; y += space) {
		sf::RectangleShape line(Vector2f(worldWidth, lineThickness));
		line.setPosition(0, y - lineThickness / 2.0f);
		line.setFillColor(lineColor);
		world.push_back(line);
	}
}

void PlayScene::HandleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed &&
		event.mouseButton.button == sf::Mouse::Left) {
		auto destination = Game::Instance().WorldMouse(Vector2i{ event.mouseButton.x, event.mouseButton.y });
		player->SetDestination(destination);
	}
}

void PlayScene::HandlePacket(const PACKET& packet)
{
	switch (packet.type)
	{
	case PACKET_TYPE::LOGIN_SUCCESS:
	{
		auto context = static_pointer_cast<LoginSuccess>(packet.context);
		player = make_unique<Player>(context->datas.front());
		entities.clear();
		for (int i = 1; i < context->datas.size(); ++i) {
			entities.emplace_back(make_unique<Player>(context->datas[i]));
			cout << context->datas[i].name << endl;
		}
	}
	break;
	case PACKET_TYPE::PLAYER_APPEND:
	{
		auto context = static_pointer_cast<PlayerAppend>(packet.context);
		cout << "append" << endl;
		entities.emplace_back(make_unique<Player>(*context));
	}
	break;
	default:
		break;
	}
}

void PlayScene::Update(const sf::Time& time)
{
	double deltaTime = time.asMicroseconds() * 1e-6;

	for (auto& entity : entities) {
		entity->Update(deltaTime);
	}
	player->Update(deltaTime);

	float halfViewWidth = Game::windowWidth / 2.0f;
	float halfViewHeight = Game::windowHeight / 2.0f;

	auto viewCenter = player->Position();

	if (viewCenter.x - halfViewWidth < 0)
		viewCenter.x = halfViewWidth;
	if (viewCenter.x + halfViewWidth > worldWidth)
		viewCenter.x = worldWidth - halfViewWidth;

	if (viewCenter.y - halfViewHeight < 0)
		viewCenter.y = halfViewHeight;
	if (viewCenter.y + halfViewHeight > worldHeight)
		viewCenter.y = worldHeight - halfViewHeight;
	view.setCenter(viewCenter);
}

void PlayScene::Render(sf::RenderWindow& window)
{
	window.setView(view);

	for (const auto& line : world) {
		window.draw(line);
	}

	for (auto& entity : entities) {
		window.draw(*entity);
	}

	window.draw(*player);
}
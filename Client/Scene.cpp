#include "Scene.h"
#include "Entity.h"
#include "Game.h"
#include "Random.h"
#include "Button.h"
#include <iostream>

using namespace std;
using namespace sf;

Scene::Scene()
	:view(FloatRect(0, 0, Game::windowWidth, Game::windowHeight))
{
	view.setSize(Game::windowWidth, Game::windowHeight);
}

PlayScene::PlayScene()
	: Scene{}
	, button{ make_unique<Button>("Restart?",
		Vector2f{Game::windowWidth / 4.f,Game::windowHeight / 4.f},
		[this]() {
			PACKET packet{ PACKET_TYPE::RESTART_TO_SERVER };
			packet.context = make_shared<RestartToServer>(player->id);
			Game::Instance().Send(packet); }) }
{
	Color lineColor{ 128,128,128 };
	float lineThickness = 2.5;
	for (int x = 0; x < worldWidth; x += space) {
		sf::RectangleShape line(sf::Vector2f(lineThickness, worldHeight));
		line.setPosition(x - lineThickness / 2.0f, 0);
		line.setFillColor(lineColor);
		world.emplace_back(line);
	}

	for (int y = 0; y < worldHeight; y += space) {
		sf::RectangleShape line(Vector2f(worldWidth, lineThickness));
		line.setPosition(0, y - lineThickness / 2.0f);
		line.setFillColor(lineColor);
		world.emplace_back(line);
	}
}

void PlayScene::HandleEvent(const sf::Event& event)
{
	if (event.type == sf::Event::MouseButtonPressed &&
		event.mouseButton.button == sf::Mouse::Left &&
		player->active) {
		auto destination = Game::Instance().WorldMouse(Vector2i{ event.mouseButton.x, event.mouseButton.y });
		player->SetDestination(destination);
		auto dir = destination - player->Position();
		PACKET packet;
		packet.type = PACKET_TYPE::PLAYER_INPUT;
		packet.context = make_shared<PlayerInput>(player->id, dir.x, dir.y);
		Game::Instance().Send(packet);
	}
	button->HandleEvent(event);
}

void PlayScene::HandlePacket(const PACKET& packet)
{
	switch (packet.type)
	{
	case PACKET_TYPE::LOGIN_SUCCESS:
	{
		auto context = static_pointer_cast<LoginSuccess>(packet.context);
		player = make_unique<Player>(context->players.front());
		view.setCenter(player->Position());
		entities.clear();
		for (const auto& info : context->foods) {
			entities.emplace_back(make_unique<Food>(info));
		}
		for (int i = 1; i < context->players.size(); ++i) {
			entities.emplace_back(make_unique<Player>(context->players[i]));
		}
		active = true;
	}
	break;
	case PACKET_TYPE::PLAYER_APPEND:
	{
		auto context = static_pointer_cast<PlayerAppend>(packet.context);
		entities.emplace_back(make_unique<Player>(*context));
	}
	break;
	case PACKET_TYPE::PLAYER_INPUT:
	{
		auto context = static_pointer_cast<PlayerInput>(packet.context);
		if (player->id == context->id) {
			player->SetDestination(context->x, context->y);
		}
		else {
			auto iter = find_if(entities.begin(), entities.end(), [&context](const auto& entity) {return entity->id == context->id; });
			if (iter != entities.end()) {
				if (Player* enemy = dynamic_cast<Player*>((*iter).get()); enemy != nullptr) {
					enemy->SetDestination(context->x, context->y);
				}
			}

		}
	}
	break;
	case PACKET_TYPE::LOGOUT:
	{
		auto context = static_pointer_cast<Logout>(packet.context);
		erase_if(entities, [&context](auto& entity) {return entity->id == context->id; });
	}
	break;
	case PACKET_TYPE::CHECK_COLLISION:
	{
		auto context = static_pointer_cast<ConfirmCollision>(packet.context);

		auto iter1 = find_if(entities.begin(), entities.end(), [&context](const auto& e) {return e->id == context->id1; });
		auto iter2 = find_if(entities.begin(), entities.end(), [&context](const auto& e) {return e->id == context->id2; });
		if (iter1 == entities.end() && iter2 == entities.end()) {
			return;
		}
		if (iter1 == entities.end() && context->id1 == player->id) {
			player->OnCollision((*iter2).get());
			(*iter2)->OnCollision(player.get());
			if (!player->active) {
				button->active = true;
				button->shape.setPosition(view.getCenter());
			}
		}
		else if (iter2 == entities.end() && context->id2 == player->id) {
			player->OnCollision((*iter1).get());
			(*iter1)->OnCollision(player.get());
			if (!player->active) {
				button->active = true;
				button->shape.setPosition(view.getCenter());
			}
		}
		else if(iter1 != entities.end() && iter2 != entities.end()){
			(*iter1)->OnCollision((*iter2).get());
			(*iter2)->OnCollision((*iter1).get());
		}
	}
	break;
	case PACKET_TYPE::RECREATE_FOOD:
	{
		auto context = static_pointer_cast<RecreateFood>(packet.context);
		for (const auto& info : context->foods) {
			auto iter = find_if(entities.begin(), entities.end(), [&info](const auto& e) {return e->id == info.id; });
			if (iter != entities.end()) {
				auto p = dynamic_cast<Food*>(iter->get());
				p->activeTime = info.activeTime;
				p->active = true;
				p->SetPosition(info.x, info.y);
			}
		}
	}
	break;
	case PACKET_TYPE::BROADCAST:
	{
		auto context = static_pointer_cast<BroadCast>(packet.context);
		for (const auto& info : context->players) {
			auto iter = find_if(entities.begin(), entities.end(), [&info](const auto& e) {return e->id == info.id; });
			if (iter != entities.end()) {
				auto& p = *iter;
				if (Vector2f::Distance(p->Position(), Vector2f{ info.x,info.y }) > Player::startSpeed * 0.1f) {
					p->shape.setPosition(info.x, info.y);
				}
			}
			else if(player->id == info.id && Vector2f::Distance(player->Position(), Vector2f{ info.x,info.y }) > Player::startSpeed * 0.1f) {
				player->shape.setPosition(info.x, info.y);
			}
		}
	}
	break;
	case PACKET_TYPE::RESTART_TO_CLIENT:
	{
		auto context = static_pointer_cast<RestartToClient>(packet.context);
		if (player->id == context->id) {
			player->active = true;
			player->SetPosition(context->x, context->y);
			player->SetColor(context->color);
			player->SetSize(Player::startSize);
		}
		else {
			auto iter = find_if(entities.begin(), entities.end(), [&context](const auto& e) {return context->id == e->id; });
			if (iter != entities.end() && dynamic_cast<Player*>((*iter).get()) != nullptr) {
				auto p = dynamic_cast<Player*>((*iter).get());
				p->SetColor(context->color);
				p->SetPosition(context->x, context->y);
				p->active = true;
				p->SetSize(Player::startSize);
			}
		}
	}
	break;
	default:
		break;
	}
}

void PlayScene::Update(const sf::Time& time)
{
	if (active) {

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
}

void PlayScene::Render(sf::RenderWindow& window)
{
	if (active) {
		window.setView(view);

		for (const auto& line : world) {
			window.draw(line);
		}

		for (auto& entity : entities) {
			window.draw(*entity);
		}

		window.draw(*player);
		window.draw(*button);
	}
}
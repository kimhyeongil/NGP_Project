#pragma once
#include "Common.h"
#include "GameCommon.h"
#include <vector>
#include <memory>
class Entity;

class Scene {
public:
	Scene();
	virtual ~Scene() = default;

	virtual void HandleEvent(const sf::Event& event) {}
	virtual void HandlePacket(const PACKET& packet) {}
	virtual void Update(const sf::Time& time) {}
	virtual void Render(sf::RenderWindow& window) {}

	sf::View view;
protected:
	std::vector<std::unique_ptr<Entity>> entities;
};

class PlayScene : public Scene {
public:
	static constexpr int worldHeight = 2000;
	static constexpr int worldWidth = 2000;
	static constexpr int space = 100;
public:
	PlayScene();

	void HandleEvent(const sf::Event& event) override;
	void HandlePacket(const PACKET& packet) override;
	void Update(const sf::Time& time) override;
	void Render(sf::RenderWindow& window) override;
private:
	std::vector<sf::RectangleShape> world;

	std::unique_ptr<Player> player;
};
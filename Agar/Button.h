#pragma once
#include <functional>
#include <string>
#include "GameCommon.h"
class Button : public sf::Drawable {
public:
	Button(const std::string& name, const sf::Vector2f& size, std::function<void()> func) 
		: onClick{ func } 
		, name{ name }
	{ 
		shape.setOutlineColor(sf::Color::Black);
		shape.setSize(size);
		shape.setOrigin(shape.getSize().x / 2, shape.getSize().y / 2);
	}

	void HandleEvent(const sf::Event& event);
	virtual void draw(sf::RenderTarget&, sf::RenderStates) const;

	bool active = false;
	sf::RectangleShape shape;
	std::function<void()> onClick;
	std::string name;
};


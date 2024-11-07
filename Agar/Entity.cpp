#include "Entity.h"

Entity::Entity()
	: shape{ 20.f }
	, position{ 400,400 }
{
	shape.setFillColor(sf::Color::Green);
	shape.setOrigin(shape.getRadius(), shape.getRadius());
}

void Entity::HandleInput(const std::vector<sf::Event>& events)
{
	for (const auto& event : events) {
		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Left) {
			position = sf::Vector2f(event.mouseButton.x, event.mouseButton.y);
		}
	}
}

void Entity::Render(sf::RenderWindow& window)
{
	shape.setPosition(position);
	window.draw(shape);
}
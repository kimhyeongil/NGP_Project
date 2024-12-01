#include "Button.h"
#include "Game.h"
#include <iostream>
using namespace sf;
using namespace std;

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const 
{
	if (active) {
		target.draw(shape, states);

		static sf::Font font;
		static bool isFontLoaded = font.loadFromFile("consola.ttf");
		if (!isFontLoaded) {
			return;
		}

		sf::Text text;
		text.setFont(font);
		text.setString(name);
		text.setCharacterSize(30);
		text.setFillColor(sf::Color::Black);

		sf::FloatRect textBounds = text.getLocalBounds();
		text.setOrigin(textBounds.worldWidth / 2.f, textBounds.worldHeight / 2.f);

		sf::Vector2f center = shape.getPosition();
		text.setPosition(center);

		target.draw(text, states);
	}
}

void Button::HandleEvent(const sf::Event& event) {
    if (active && event.type == sf::Event::MouseButtonPressed) {
		sf::Vector2f mousePos = Game::Instance().WorldMouse(Vector2i{ event.mouseButton.x, event.mouseButton.y });

        sf::FloatRect buttonBounds = shape.getGlobalBounds();

        if (buttonBounds.contains(mousePos.x, mousePos.y)) {
            onClick();
			active = false;
        }
    }
}
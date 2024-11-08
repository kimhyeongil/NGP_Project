#pragma once
#include <array>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

#undef min
#undef max

const std::array<sf::Color, 3> colors{ sf::Color::Red, sf::Color::Green, sf::Color::Blue };

enum PACKET_TYPE : unsigned int
{
	PLAYER_INPUT = 1
};
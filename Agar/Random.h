#pragma once
#include "Common.h"
#include <random>
struct Random
{
	static int RandInt(int min, int max);
	static float RandRange();
	static sf::Color RandColor();

	static std::random_device rd;
	static std::default_random_engine dre;
};


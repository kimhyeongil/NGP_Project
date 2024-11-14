#include "Random.h"
#include "GameCommon.h"

using namespace std;

random_device Random::rd;
default_random_engine Random::dre(rd());

int Random::RandInt(int min, int max)
{
	uniform_int_distribution uid(min, max);
	return uid(dre);
}

float Random::RandRange()
{
	static uniform_real_distribution urd(0.f, 1.f);
	return urd(dre);
}

sf::Color Random::RandColor()
{
	static uniform_int_distribution uid(0, 255);
	return sf::Color(uid(dre), uid(dre), uid(dre));
}
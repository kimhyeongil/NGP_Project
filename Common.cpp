#include "Common.h"
#include "Entity.h"

LoginSuccess::PlayerInfo::PlayerInfo(const Player& player)
{
	id = player.id;
	color = player.color;
	size = player.size;
	x = player.Position().x;
	y = player.Position().y;
	memcpy(name, player.name, 16);
}

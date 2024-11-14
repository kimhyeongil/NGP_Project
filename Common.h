#pragma once
#include <winsock2.h>

#undef min
#undef max

using uint = unsigned int;

enum PACKET_TYPE : uint
{
	PLAYER_INPUT = 1
};

struct PACKET {
	uint type;

	void* context;
};


struct PlayerInput {
	PlayerInput(int id = 0, float x = 0, float y = 0) :id{ id }, x{ x }, y{ y } {}

	void ntoh() 
	{ 
		id = ntohl(id);
		uint tempx; memcpy(&tempx, &x, sizeof(float)); x = ntohf(tempx);  
		uint tempy; memcpy(&tempy, &y, sizeof(float)); y = ntohf(tempy);
	}
	void hton() 
	{ 
		id = htonl(id);
		uint tempx = htonf(x); memcpy(&x, &tempx, sizeof(float)); 
		uint tempy = htonf(y); memcpy(&y, &tempy, sizeof(float));
	}
	int id;
	float x, y;
};
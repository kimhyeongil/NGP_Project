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
	PlayerInput(float x = 0, float y = 0) : x{ x }, y{ y } {}

	void ntoh() 
	{ 
		uint tempx; memcpy(&tempx, &x, sizeof(float)); x = ntohf(tempx);  
		uint tempy; memcpy(&tempy, &y, sizeof(float)); y = ntohf(tempy);
	}
	void hton() 
	{ 
		uint tempx = htonf(x); memcpy(&x, &tempx, sizeof(float)); 
		uint tempy = htonf(y); memcpy(&y, &tempy, sizeof(float));
	}
	float x, y;
};
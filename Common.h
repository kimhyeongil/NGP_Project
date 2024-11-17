#pragma once
#include <winsock2.h>
#include <memory>
#include <any>

#undef min
#undef max

using uint = unsigned int;

enum PACKET_TYPE : uint
{
//<<<<<<< HEAD
	PLAYER_INPUT = 1
	, LOGIN_SUCCESS
	,LOGIN_TRY
,
	PLAYER_APPEND
//>>>>>>> main
};

struct PACKET {
	uint type;

	std::any context;
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

struct PlayerAppend {
	void ntoh()
	{
		id = ntohl(id);
		color = ntohl(color);
		uint tempx; memcpy(&tempx, &x, sizeof(float)); x = ntohf(tempx);
		uint tempy; memcpy(&tempy, &y, sizeof(float)); y = ntohf(tempy);
	}
	void hton()
	{
		id = htonl(id);
		color = htonl(color);
		uint tempx = htonf(x); memcpy(&x, &tempx, sizeof(float));
		uint tempy = htonf(y); memcpy(&y, &tempy, sizeof(float));
	}

	int id;
	int color;
	float x, y;
};
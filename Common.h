#pragma once
#include <winsock2.h>
#include <memory>
#include <any>

#undef min
#undef max

using uint = unsigned int;

struct PacketContext {
	virtual void Send(SOCKET sock) = 0;
	virtual void Recv(SOCKET sock) = 0;
	virtual void ntoh() = 0;
	virtual void hton() = 0;
};

struct PlayerInput :public PacketContext {
	PlayerInput(int id = 0, float x = 0, float y = 0) :id{ id }, x{ x }, y{ y } {}

	void Send(SOCKET sock) override
	{
		PlayerInput temp = *this;
		temp.hton();
		send(sock, (char*)&temp, sizeof(PlayerInput), 0);
	}

	void Recv(SOCKET sock) override
	{
		PlayerInput temp;
		recv(sock, (char*)&temp, sizeof(PlayerInput), 0);
		*this = temp;
		ntoh();
	}

	void ntoh() override
	{ 
		id = ntohl(id);
		uint tempx; memcpy(&tempx, &x, sizeof(float)); x = ntohf(tempx);  
		uint tempy; memcpy(&tempy, &y, sizeof(float)); y = ntohf(tempy);
	}

	void hton() override
	{ 
		id = htonl(id);
		uint tempx = htonf(x); memcpy(&x, &tempx, sizeof(float)); 
		uint tempy = htonf(y); memcpy(&y, &tempy, sizeof(float));
	}
	int id;
	float x, y;
};

struct PlayerAppend : public PacketContext{
	void Send(SOCKET sock) override
	{
		PlayerAppend temp = *this;
		temp.hton();
		send(sock, (char*)&temp, sizeof(PlayerAppend), 0);
	}

	void Recv(SOCKET sock) override
	{
		PlayerAppend temp;
		recv(sock, (char*)&temp, sizeof(PlayerAppend), 0);
		*this = temp;
		ntoh();
	}

	void ntoh() override
	{
		id = ntohl(id);
		color = ntohl(color);
		uint tempx; memcpy(&tempx, &x, sizeof(float)); x = ntohf(tempx);
		uint tempy; memcpy(&tempy, &y, sizeof(float)); y = ntohf(tempy);
	}

	void hton() override
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

enum PACKET_TYPE : uint
{

	PLAYER_INPUT = 1
	, LOGIN_SUCCESS
	, LOGIN_TRY
	,
	PLAYER_APPEND

};

struct PACKET {
	uint type;

	std::shared_ptr<PacketContext> context;
};
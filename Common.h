#pragma once
#include <winsock2.h>
#include <memory>
#include <vector>

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
	char name[16];
};

struct LoginSuccess : public PacketContext {
	struct PlayerInfo {
		PlayerInfo() = default;
		PlayerInfo(const class Player&);
		void ntoh()
		{
			id = ntohl(id);
			color = ntohl(color);
			uint tempX; memcpy(&tempX, &x, sizeof(float)); x = ntohf(tempX);
			uint tempY; memcpy(&tempY, &y, sizeof(float)); y = ntohf(tempY);
			uint tempSize; memcpy(&tempSize, &size, sizeof(float)); size = ntohf(tempSize);
		}

		void hton()
		{
			id = htonl(id);
			color = htonl(color);
			uint tempX = htonf(x); memcpy(&x, &tempX, sizeof(float));
			uint tempY = htonf(y); memcpy(&y, &tempY, sizeof(float));
			uint tempSize = htonf(size); memcpy(&size, &tempSize, sizeof(float));
		}

		int id;
		char name[16];
		float x, y;
		int color;
		float size;
	};

	void Send(SOCKET sock) override
	{
		uint size = datas.size();
		size = htonl(size);
		send(sock, (char*)&size, sizeof(uint), 0);

		hton();
		send(sock, (char*)datas.data(), datas.size() * sizeof(PlayerInfo), 0);
	}

	void Recv(SOCKET sock) override
	{
		uint size;
		recv(sock, (char*)&size, sizeof(uint), 0);
		size = ntohl(size);

		datas.resize(size);
		recv(sock, (char*)datas.data(), datas.size() * sizeof(PlayerInfo), 0);
		ntoh();
	}

	void ntoh() override
	{
		for (auto& data : datas) {
			data.ntoh();
		}
	}

	void hton() override
	{
		for (auto& data : datas) {
			data.hton();
		}
	}

	std::vector<PlayerInfo> datas;
};

enum PACKET_TYPE : uint
{

	PLAYER_INPUT = 1
	,LOGIN_SUCCESS
	,PLAYER_APPEND

};

struct PACKET {
	uint type;

	std::shared_ptr<PacketContext> context;
};
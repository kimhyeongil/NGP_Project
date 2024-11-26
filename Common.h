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
	struct Data {
		Data() = default;
		Data(const PlayerInput& context) : id{ context.id }, x{ context.x }, y{ context.y } {}
		int id;
		float x, y;
	};

	PlayerInput(int id = 0, float x = 0, float y = 0) : id{ id }, x{ x }, y{ y } {}

	PlayerInput& operator=(const Data& data)
	{
		id = data.id;
		x = data.x;
		y = data.y;
		return *this;
	}

	void Send(SOCKET sock) override
	{
		PlayerInput temp = *this;
		temp.hton();
		Data data = temp;
		send(sock, (char*)&data, sizeof(Data), 0);
	}

	void Recv(SOCKET sock) override
	{
		Data data;
		recv(sock, (char*)&data, sizeof(Data), 0);
		*this = data;
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
	struct Data {
		Data() = default;
		Data(const PlayerAppend& context) : id{ context.id }, x{ context.x }, y{ context.y }, color{ context.color } { memcpy(name, context.name, 16); }
		int id;
		int color;
		float x, y;
		char name[16];
	};

	PlayerAppend& operator=(const Data& data)
	{
		id = data.id;
		x = data.x;
		y = data.y;
		color = data.color;
		memcpy(name, data.name, 16);
		return *this;
	}

	void Send(SOCKET sock) override
	{
		PlayerAppend temp = *this;
		temp.hton();
		Data data = temp;
		send(sock, (char*)&data, sizeof(Data), 0);
	}

	void Recv(SOCKET sock) override
	{
		Data data;
		recv(sock, (char*)&data, sizeof(Data), 0);
		*this = data;
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

struct ConfirmCollision : public PacketContext {
	struct Data {
		Data() = default;
		Data(const ConfirmCollision& context) : id1{ context.id1 }, id2{ context.id2 } {}
		int id1;
		int id2;
	};

	ConfirmCollision& operator=(const Data& data)
	{
		id1 = data.id1;
		id2 = data.id2;
		return *this;
	}

	void Send(SOCKET sock) override
	{
		ConfirmCollision temp = *this;
		temp.hton();
		Data data = temp;
		send(sock, (char*)&data, sizeof(Data), 0);
	}

	void Recv(SOCKET sock) override
	{
		Data data;
		recv(sock, (char*)&data, sizeof(Data), 0);
		*this = data;
		ntoh();
	}

	void ntoh() override
	{
		id1 = ntohl(id1);
		id2 = ntohl(id2);
	}

	void hton() override
	{
		id1 = htonl(id1);
		id2 = htonl(id2);
		
	}
	int id1;
	int id2;
};

struct FoodInfo {
	FoodInfo() = default;
	FoodInfo(const class Food&);

	void ntoh()
	{
		id = ntohl(id);
		uint tempX; memcpy(&tempX, &x, sizeof(float)); x = ntohf(tempX);
		uint tempY; memcpy(&tempY, &y, sizeof(float)); y = ntohf(tempY);
		uint tempTime; memcpy(&tempTime, &activeTime, sizeof(float)); activeTime = ntohf(tempTime);
	}

	void hton()
	{
		id = htonl(id);
		uint tempX = htonf(x); memcpy(&x, &tempX, sizeof(float));
		uint tempY = htonf(y); memcpy(&y, &tempY, sizeof(float));
		uint tempTime = htonf(activeTime); memcpy(&activeTime, &tempTime, sizeof(float));
	}

	int id;
	float x, y;
	float activeTime;
};

struct PlayerInfo {
	PlayerInfo() = default;
	PlayerInfo(const class Player&);

	void ntoh()
	{
		id = ntohl(id);
		color = ntohl(color);
		size = ntohl(size);
		uint tempX; memcpy(&tempX, &x, sizeof(float)); x = ntohf(tempX);
		uint tempY; memcpy(&tempY, &y, sizeof(float)); y = ntohf(tempY);
	}

	void hton()
	{
		id = htonl(id);
		color = htonl(color);
		size = htonl(size);
		uint tempX = htonf(x); memcpy(&x, &tempX, sizeof(float));
		uint tempY = htonf(y); memcpy(&y, &tempY, sizeof(float));
	}

	int id;
	char name[16];
	float x, y;
	int color;
	int size;
};

struct LoginSuccess : public PacketContext {

	void Send(SOCKET sock) override
	{
		hton();

		uint size = players.size();
		size = htonl(size);
		send(sock, (char*)&size, sizeof(uint), 0);

		send(sock, (char*)players.data(), players.size() * sizeof(PlayerInfo), 0);

		size = foods.size();
		size = htonl(size);
		send(sock, (char*)&size, sizeof(uint), 0);

		send(sock, (char*)foods.data(), foods.size() * sizeof(FoodInfo), 0);
	}

	void Recv(SOCKET sock) override
	{
		uint size;
		recv(sock, (char*)&size, sizeof(uint), 0);
		size = ntohl(size);

		players.resize(size);
		recv(sock, (char*)players.data(), players.size() * sizeof(PlayerInfo), 0);

		recv(sock, (char*)&size, sizeof(uint), 0);
		size = ntohl(size);

		foods.resize(size);
		recv(sock, (char*)foods.data(), foods.size() * sizeof(FoodInfo), 0);

		ntoh();
	}

	void ntoh() override
	{
		for (auto& player : players) {
			player.ntoh();
		}

		for (auto& food : foods) {
			food.ntoh();
		}
	}

	void hton() override
	{
		for (auto& player : players) {
			player.hton();
		}

		for (auto& food : foods) {
			food.hton();
		}
	}

	std::vector<PlayerInfo> players;
	std::vector<FoodInfo> foods;
};

struct Logout : public PacketContext {
	void Send(SOCKET sock) override
	{
		int data = htonl(id);
		send(sock, (char*)&data, sizeof(int), 0);
	}

	void Recv(SOCKET sock) override
	{
		int data;
		recv(sock, (char*)&data, sizeof(int), 0);
		id = ntohl(data);
	}

	void ntoh() override {}

	void hton() override {}
	int id;
};

enum PACKET_TYPE : uint
{

	PLAYER_INPUT = 1
	,LOGIN_SUCCESS
	,PLAYER_APPEND
	,LOGOUT
	, CHECK_COLLISION

};

struct PACKET {
	uint type;

	std::shared_ptr<PacketContext> context;
};
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

};

struct PACKET {
	uint type;

	std::shared_ptr<PacketContext> context;
};
#pragma once
#include <memory>
#include <winsock2.h>

using uint = unsigned int;

enum class CMD_TYPE : uint {
	LOGIN_SUCCESS = 1,
	PLAYER_APPEND,
	PLAYER_INPUT,
	LOGOUT,
	CHECK_COLLISION,
	RECREATE_FOOD,
	BROADCAST,
};

struct Command {
	Command(CMD_TYPE type) : type{ type } {}
	CMD_TYPE type;

	std::shared_ptr<void> context;
};

struct CMD_PlayerAppend {
	CMD_PlayerAppend(SOCKET sock, float x, float y, int color, const char* name)
		:appendSock{ sock }
		, x{ x }
		, y{ y }
		, color{ color }
	{
		memcpy(this->name, name, 16);
	}

	SOCKET appendSock;
	
	float x, y;
	int color;
	char name[16];
};
struct CMD_BroadCast {
};
struct CMD_LoginSuccess {
	SOCKET appendSock;
	char name[16];
};

struct CMD_PlayerInput {
	int id;
	float x, y;
};

struct CMD_Logout {
	SOCKET outSock;
};

struct CMD_CheckCollision {
	int id1;
	int id2;
};
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
};

struct Command {
	Command(CMD_TYPE type) : type{ type } {}
	CMD_TYPE type;

	std::shared_ptr<void> context;
};

struct CMD_PlayerAppend {
	SOCKET appendSock;
};

struct CMD_LoginSuccess {
	SOCKET appendSock;
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
#pragma once
#include <memory>
#include <winsock2.h>

using uint = unsigned int;

enum class CMD_TYPE : uint {
	LOGIN_SUCCESS = 1,
	PLAYER_APPEND
};

struct Command {
	CMD_TYPE type;

	std::shared_ptr<void> context;
};

struct CMD_PlayerAppend {
	int id;
	SOCKET appendSock;
};

struct CMD_LoginSuccess {
	CMD_LoginSuccess() = default;
	CMD_LoginSuccess(SOCKET sock) : appendSock{ sock } {}

	SOCKET appendSock;
};
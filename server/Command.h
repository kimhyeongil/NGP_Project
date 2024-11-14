#pragma once
#include <memory>
#include <any>

using uint = unsigned int;

enum COMMAND_TYPE : uint {
	CMD_PLAYER_APPEND = 1
};

struct Command {
	COMMAND_TYPE type;

	std::any context;
};
#include "Game.h"

#pragma comment(lib, "ws2_32")

int main() {
	if (!Game::Instance().Init("127.0.0.1", 9000)) {
		return -1;
	}
	Game::Instance().Run();
}

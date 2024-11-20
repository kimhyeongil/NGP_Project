#pragma once
#include <queue>
#include <mutex>
#include <list>
#include <condition_variable>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <array>
#include "Command.h"
#include "Entity.h"

class Server {
private:
	enum MUTEX_CATEGORY : size_t {
		  EXCUTE
		, IO
		, CLIENT_SOCK
		, ENTITIES
		, SIZE
	};
public:
	Server();
	~Server();

	void Run();

private:
	void AcceptClient();
	void ProcessClient(SOCKET client_sock);
	void Excute();

	void CheckCollision();
	void Update(double deltaTime);
private:
	SOCKET listen_sock;

	std::array<std::mutex, MUTEX_CATEGORY::SIZE> mutexes;

	std::condition_variable cv;

	std::queue<std::unique_ptr<Command>> excuteQueue;
	std::list<SOCKET> clients;

	// 나중에 먹이 추가 시 그에 맞게 변경할 예정
	std::list<std::unique_ptr<Player>> players;
	int newID = 0;
};


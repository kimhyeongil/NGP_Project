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
		, ENTITIES
		, SIZE
	};
	static constexpr float recreateTime = 5;
	static constexpr int maxReCnt = 20;
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

	std::list<std::unique_ptr<Player>> players;
	std::vector<std::unique_ptr<Food>> foods;
	
	float recreateDeltaTime = 0;
};


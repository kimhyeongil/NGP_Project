#pragma once
#include <queue>
#include <mutex>
#include <list>
#include <condition_variable>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Command.h"

class Server {
public:
	Server();
	~Server();

	void Run();

	void AcceptClient();
	void ProcessClient(SOCKET client_sock);
	void Excute();
private:
	SOCKET listen_sock;

	std::mutex excuteMtx;
	std::mutex clientMtx;
	std::mutex ioMtx;

	std::condition_variable cv;

	std::queue<Command> excuteQueue;
	std::list<SOCKET> clients;

	//�÷��̾� ���̵� �ִµ� ���� �÷��̾�� ���� ����
	std::list<int> playerIDs;
};


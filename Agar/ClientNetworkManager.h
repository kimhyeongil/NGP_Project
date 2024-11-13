#pragma once
#include <queue>
#include "../Common.h"
#include <mutex>
#include <condition_variable>

class ClientNetworkManager {
public:
	ClientNetworkManager();
	~ClientNetworkManager();

	void AddPacket(PACKET packet);
	
	void Send();
	void Recv();
private:
	SOCKET sock;
	std::queue<PACKET> sendQueue;
	std::vector<PACKET> recvPacket;

	std::condition_variable cv;
	std::mutex mtx;
};


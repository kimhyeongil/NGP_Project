#pragma once
#include <queue>
#include <list>
#include <mutex>
#include <condition_variable>
#include "Common.h"

class ClientNetworkManager {
public:
	ClientNetworkManager();
	~ClientNetworkManager();

	void AddPacket(PACKET packet);
	std::vector<PACKET> GetPacket();

	void Send();
	void Recv();
private:
	SOCKET sock;
	std::queue<PACKET> sendQueue;
	std::list<PACKET> recvPacket;

	std::condition_variable cv;
	std::mutex sendMtx;
	std::mutex recvMtx;
};


#include "ClientNetworkManager.h"
#include <thread>
#include <iostream>
using namespace std;
ClientNetworkManager::ClientNetworkManager()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit(-1);
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		WSACleanup();
		exit(-1);
	}

	string serverIP;
	cout << "서버 IP: ";
	cin >> serverIP;
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
	serveraddr.sin_port = htons(9000);

	if (connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
		closesocket(sock);
		WSACleanup();
		exit(-1);
	}
	thread(&ClientNetworkManager::Send, this).detach();
	thread(&ClientNetworkManager::Recv, this).detach();
}

ClientNetworkManager::~ClientNetworkManager()
{
	closesocket(sock);
	WSACleanup();
}

vector<PACKET> ClientNetworkManager::GetPacket()
{
	lock_guard<mutex> lock(recvMtx);
	vector<PACKET> ret;
	ret.reserve(recvPacket.size());
	copy(recvPacket.begin(), recvPacket.end(), back_inserter(ret));
	recvPacket.clear();
	return ret;
}

void ClientNetworkManager::AddPacket(PACKET packet)
{
	unique_lock<mutex> lock(sendMtx);
	sendQueue.emplace(packet);
	cv.notify_all();
}

void ClientNetworkManager::Send()
{
	while (true) {
		unique_lock<mutex> lock(sendMtx);
		cv.wait(lock, [&] {return !sendQueue.empty(); });

		auto packet = sendQueue.front();
		sendQueue.pop();
		lock.unlock();

		uint netType = htonl(packet.type);
		if (send(sock, (char*)&netType, sizeof(packet.type), 0) == SOCKET_ERROR) {
			exit(-1);
		}

		if (packet.type == PACKET_TYPE::PLAYER_INPUT) {
			auto netContext = any_cast<PlayerInput>(packet.context);
			netContext.hton();
			if (send(sock, (char*)&netContext, sizeof(PlayerInput), 0) == SOCKET_ERROR) {
				exit(-1);
			}
		}
	}
}

void ClientNetworkManager::Recv()
{
	while (true) {
		PACKET packet;
		if (recv(sock, (char*)&packet.type, sizeof(uint), 0) == SOCKET_ERROR) {
			exit(10000);
		}
		packet.type = ntohl(packet.type);
		switch (packet.type) {
		case PACKET_TYPE::PLAYER_APPEND:
		{
			PlayerAppend context;
			if (recv(sock, (char*)&context, sizeof(PlayerAppend), 0) == SOCKET_ERROR) {
				exit(10000);
			}
			context.ntoh();
			packet.context = context;
			cout << "네트워크 매니저 " << context.x << ", " << context.y << endl;
		}
		break;
		default:
			break;
		}

		lock_guard<mutex> lock(recvMtx);
		recvPacket.emplace_back(packet);
	}
}
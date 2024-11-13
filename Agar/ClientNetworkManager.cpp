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
	cout << "¼­¹ö IP: ";
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

void ClientNetworkManager::AddPacket(PACKET packet)
{
	unique_lock<mutex> lock(mtx);
	sendQueue.push(packet);
	cv.notify_all();
}

void ClientNetworkManager::Send()
{
	while (true) {
		unique_lock<mutex> lock(mtx);
		cv.wait(lock, [&] {return !sendQueue.empty(); });

		auto packet = sendQueue.front();
		sendQueue.pop();
		lock.unlock();

		uint netType = htonl(packet.type);
		send(sock, (char*)&netType, sizeof(packet.type), 0);

		if (packet.type == PACKET_TYPE::PLAYER_INPUT) {
			auto netContext = *(PlayerInput*)packet.context;
			netContext.hton();
			send(sock, (char*)&netContext, sizeof(PlayerInput), 0);
		}
	}
}

void ClientNetworkManager::Recv()
{

}
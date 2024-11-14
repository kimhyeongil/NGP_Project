#include <print>
#include <iostream>
#include "Server.h"
#include "../Common.h"

using namespace std;
Server::Server()
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		exit(-1);
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_sock == INVALID_SOCKET) {
		WSACleanup();
		exit(-1);
	}
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(9000);

	if (bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
		closesocket(listen_sock);
		WSACleanup();
		exit(-1);
	}

	if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(listen_sock);
		WSACleanup();
		exit(-1);
	}

	thread(&Server::Excute, this).detach();
	thread(&Server::AcceptClient, this).detach();
}

Server::~Server()
{
	closesocket(listen_sock);
	WSACleanup();
}

void Server::AcceptClient()
{
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);

	while (true) {
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			break;
		}
		clients.emplace_back(client_sock);
		thread(&Server::ProcessClient, this, client_sock).detach();
	}
}

void Server::Run()
{
	// 업데이트, 충돌 처리 등 필요
	while (true);
}

void Server::ProcessClient(SOCKET client_sock)
{
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	unique_lock<mutex> ioLock(ioMtx);
	print("[TCP 서버] 클라이언트 접속 : IP 주소 = {}, 포트 번호 = {}\n", addr, ntohs(clientaddr.sin_port));
	ioLock.unlock();

	unique_lock<mutex> clientLock(clientMtx);
	playerIDs.emplace_back(playerIDs.size());
	Command cmd;
	cmd.type = COMMAND_TYPE::CMD_PLAYER_APPEND;
	cmd.context = (int)playerIDs.size();
	clientLock.unlock();

	unique_lock<mutex> excuteLock(excuteMtx);


	excuteQueue.emplace(cmd);
	cv.notify_all();
	excuteLock.unlock();

	uint type;
	while (true) {
		if (recv(client_sock, (char*)&type, sizeof(uint), MSG_WAITALL) == SOCKET_ERROR) {
			break;
		}
		type = ntohl(type);

		// 서버가 클라에게 받아야 할 내용 PLAYER_INPUT 수신은 완 PLAYER_INPUT 재송신 필요
		switch (type) {
		case PACKET_TYPE::PLAYER_INPUT: 
		{
			PlayerInput input;
			if (recv(client_sock, (char*)&input, sizeof(PlayerInput), MSG_WAITALL) == SOCKET_ERROR) {
				break;
			}
			input.ntoh();
			ioLock.lock();
			print("[TCP 서버] 클라이언트 입력: IP 주소 = {}, 포트 번호 = {}, 위치 ({}, {})\n", addr, ntohs(clientaddr.sin_port), input.x, input.y);
			ioLock.unlock();
		}
		break;
		default:
			break;
		}
	}

	ioLock.lock();
	print("[TCP 서버] 클라이언트 종료 : IP 주소 = {}, 포트 번호 = {}\n", addr, ntohs(clientaddr.sin_port));
}

void Server::Excute()
{
	while (true) {
		unique_lock<mutex> lock(excuteMtx);
		cv.wait(lock, [&] {return !excuteQueue.empty(); });

		auto command = move(excuteQueue.front());
		excuteQueue.pop();
		lock.unlock();

		switch (command.type)
		{
		case COMMAND_TYPE::CMD_PLAYER_APPEND:
		{
			auto playerID = any_cast<int>(command.context);
			uint type = PACKET_TYPE::PLAYER_APPEND;
			type = htonl(type);
			PlayerAppend context;
			context.id = playerID;
			//랜덤한 값으로 수정
			context.x = 100;
			context.y = 100;
			context.color = 0;
			context.hton();
			lock_guard<mutex> clientLock(clientMtx);
			for (auto& client : clients) {
				send(client, (char*)&type, sizeof(type), 0);
				send(client, (char*)&context, sizeof(context), 0);
			}
		}
		break;
		default:
			break;
		}
	}
}
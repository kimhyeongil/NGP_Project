#include <print>
#include <iostream>
#include <algorithm>
#include "Server.h"
#include "Common.h"
#include "Random.h"
#include "Scene.h"

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
	while (true)
	{
		Update(1);
		CheckCollision();
	}
}

void Server::CheckCollision()
{

}

void Server::Update(double deltaTime)
{

}

void Server::ProcessClient(SOCKET client_sock)
{
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	unique_lock<mutex> ioLock(mutexes[IO]);
	print("[TCP 서버] 클라이언트 접속 : IP 주소 = {}, 포트 번호 = {}\n", addr, ntohs(clientaddr.sin_port));
	ioLock.unlock();

	unique_lock<mutex> clientLock(mutexes[CLIENT_SOCK]);
	clients.emplace_back(client_sock);
	auto cmd = make_unique<Command>();
	cmd->type = CMD_TYPE::LOGIN_SUCCESS;
	auto context = make_shared<CMD_LoginSuccess>(client_sock);
	cmd->context = context;
	clientLock.unlock();


	unique_lock<mutex> excuteLock(mutexes[EXCUTE]);
	excuteQueue.emplace(move(cmd));
	cv.notify_all();
	excuteLock.unlock();

	uint type;
	while (true) {
		//if (recv(client_sock, (char*)&type, sizeof(uint), MSG_WAITALL) == SOCKET_ERROR) {
		//	break;
		//}
		//type = ntohl(type);

		//// 서버가 클라에게 받아야 할 내용 PLAYER_INPUT 수신은 완 PLAYER_INPUT 재송신 필요
		//switch (type) {
		//case PACKET_TYPE::PLAYER_INPUT: 
		//{
		//	PlayerInput input;
		//	if (recv(client_sock, (char*)&input, sizeof(PlayerInput), MSG_WAITALL) == SOCKET_ERROR) {
		//		break;
		//	}
		//	input.ntoh();
		//	ioLock.lock();
		//	print("[TCP 서버] 클라이언트 입력: IP 주소 = {}, 포트 번호 = {}, 위치 ({}, {})\n", addr, ntohs(clientaddr.sin_port), input.x, input.y);
		//	ioLock.unlock();
		//}
		//break;
		//default:
		//	break;
		//}
	}

	ioLock.lock();
	print("[TCP 서버] 클라이언트 종료 : IP 주소 = {}, 포트 번호 = {}\n", addr, ntohs(clientaddr.sin_port));
}

void Server::Excute()
{
	while (true) {
		unique_lock<mutex> lock(mutexes[EXCUTE]);
		cv.wait(lock, [&] {return !excuteQueue.empty(); });
		auto cmd = move(excuteQueue.front());
		excuteQueue.pop();
		lock.unlock();
		
		switch (cmd->type)
		{
		case CMD_TYPE::LOGIN_SUCCESS:
		{
			auto context = static_pointer_cast<CMD_LoginSuccess>(cmd->context);
			auto player = make_unique<Player>();
			player->id = newID++;
			auto newCMD = make_unique<Command>();
			auto newContext = make_shared<CMD_PlayerAppend>(context->appendSock, player->id);
			newCMD->context = newContext;
			newCMD->type = CMD_TYPE::PLAYER_APPEND;
			player->color = Random::RandInt(0, colors.size() - 1);
			//sf::Vector2f pos(Random::RandInt(Player::startSize, PlayScene::worldWidth - Player::startSize), Random::RandInt(Player::startSize, PlayScene::worldHeight - Player::startSize));
			sf::Vector2f pos(Random::RandInt(100, 150), Random::RandInt(100, 150));

			player->SetPosition(pos);
			// 나중에 로그인 시 받은 이름으로 변경할 예정
			unique_lock<mutex> entityLock(mutexes[ENTITIES]);
			string name = "Player" + to_string(players.size());
			memcpy(player->name, name.c_str(), name.length());
			vector<LoginSuccess::PlayerInfo> datas;
			datas.emplace_back(*player);
			for (const auto& player : players) {
				datas.emplace_back(*player);
			}
			players.emplace_front(move(player));
			entityLock.unlock();
			auto packet = make_shared<LoginSuccess>();
			packet->datas = datas;
			uint type = (uint)PACKET_TYPE::LOGIN_SUCCESS;
			type = htonl(type);
			send(context->appendSock, (char*)&type, sizeof(uint), 0);
			packet->Send(context->appendSock);
			lock.lock();
			excuteQueue.emplace(move(newCMD));
			lock.unlock();
		}
		break;
		case CMD_TYPE::PLAYER_APPEND:
		{
			cout << excuteQueue.size() << endl;
			auto context = static_pointer_cast<CMD_PlayerAppend>(cmd->context);
			auto packet = make_shared<PlayerAppend>();
			unique_lock<mutex> lock(mutexes[ENTITIES]);
			auto iter = find_if(players.begin(), players.end(), [&](const auto& player) {return player->id == context->id; });
			if (iter != players.end()) {
				const auto& player = *iter;
				packet->color = player->color;
				packet->id = player->id;
				packet->x = player->Position().x;
				packet->y = player->Position().y;
				memcpy(packet->name, player->name, 16);
				lock.unlock();
				uint type = htonl(PACKET_TYPE::PLAYER_APPEND);
				for (auto sock : clients) {
					if (sock != context->appendSock) {
						send(sock, (char*)&type, sizeof(uint), 0);
						packet->Send(sock);
					}
				}
			}
			else {
				lock.unlock();
			}
		}
		break;
		default:
			break;
		}
	}
}
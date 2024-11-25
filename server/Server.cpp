#include <print>
#include <iostream>
#include <algorithm>
#include <chrono>
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
		thread(&Server::ProcessClient, this, client_sock).detach();
	}
}

void Server::Run()
{
	// ������Ʈ, �浹 ó�� �� �ʿ�
	auto start = chrono::high_resolution_clock::now();
	while (true)
	{
		auto end = chrono::high_resolution_clock::now();
		double deltaTime = chrono::duration_cast<chrono::nanoseconds>(end - start).count() * 1e-9;
		while (deltaTime < 1e-4) {
			end = chrono::high_resolution_clock::now();
			deltaTime = chrono::duration_cast<chrono::nanoseconds>(end - start).count() * 1e-9;
		}
		start = end;
		Update(deltaTime);
		CheckCollision();
	}
}

void Server::CheckCollision() //���� ���̱����� �浹üũ �������
{
	lock_guard<mutex> entityLock(mutexes[ENTITIES]);

	// �浹�� �÷��̾� �� �����͸� ����
	for (auto it1 = players.begin(); it1 != players.end(); ++it1) {
		for (auto it2 = next(it1); it2 != players.end(); ) {
			auto& player1 = *it1;
			auto& player2 = *it2;

			// �� �÷��̾� �� �Ÿ� ���
			auto dx = player2->Position().x - player1->Position().x;
			auto dy = player2->Position().y - player1->Position().y;
			float distance = sqrt(dx * dx + dy * dy);

			float combinedRadius = player1->size + player2->size;
			
			if (distance < combinedRadius) {
				//printf("%f, %f\n", distance, combinedRadius);
				// �浹 �߻�, CMD_CheckCollision ����
				auto cmd = make_unique<Command>(CMD_TYPE::CHECK_COLLISION); // CMD_TYPE ���� �ʿ�
				auto context = make_shared<CMD_CheckCollision>(player1->id, player2->id);
				cmd->context = context;

				// Excute()�� ����
				lock_guard<mutex> executeLock(mutexes[EXCUTE]);
				excuteQueue.emplace(move(cmd));
				cv.notify_all();

				// it2�� ������Ű�� �ʰ� ��� ������ ���� ���ѷ����� �߻��մϴ�.
				++it2;
			}
			else {
				++it2;
			}
		}
	}
}


void Server::Update(double deltaTime)
{
	lock_guard<mutex> entityLock(mutexes[ENTITIES]);
	for (auto& player : players) {
		player->Update(deltaTime);
	}
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
	print("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = {}, ��Ʈ ��ȣ = {}\n", addr, ntohs(clientaddr.sin_port));
	ioLock.unlock();

	unique_lock<mutex> clientLock(mutexes[CLIENT_SOCK]);
	clients.emplace_back(client_sock);
	clientLock.unlock();

	auto cmd = make_unique<Command>(CMD_TYPE::LOGIN_SUCCESS);
	cmd->context = make_shared<CMD_LoginSuccess>(client_sock);
	unique_lock<mutex> excuteLock(mutexes[EXCUTE]);
	excuteQueue.emplace(move(cmd));
	cv.notify_all();
	excuteLock.unlock();

	uint type;
	while (true) {
		if (recv(client_sock, (char*)&type, sizeof(uint), MSG_WAITALL) == SOCKET_ERROR) {
			break;
		}
		type = ntohl(type);

		// ������ Ŭ�󿡰� �޾ƾ� �� ���� PLAYER_INPUT ������ �� PLAYER_INPUT ��۽� �ʿ�
		switch (type) {
		case PACKET_TYPE::PLAYER_INPUT: 
		{
			auto input = make_shared<PlayerInput>();
			input->Recv(client_sock);
			ioLock.lock();
			print("[TCP ����] Ŭ���̾�Ʈ �Է�: IP �ּ� = {}, ��Ʈ ��ȣ = {}, ��ġ ({}, {})\n", addr, ntohs(clientaddr.sin_port), input->x, input->y);
			ioLock.unlock();
			auto cmd = make_unique<Command>(CMD_TYPE::PLAYER_INPUT);
			auto context = make_shared<CMD_PlayerInput>();
			context->id = input->id;
			context->x = input->x;
			context->y = input->y;
			cmd->context = context;
			excuteLock.lock();
			excuteQueue.emplace(move(cmd));
			cv.notify_all();
			excuteLock.unlock();
		}
		break;
		default:
			break;
		}
	}

	ioLock.lock();
	print("[TCP ����] Ŭ���̾�Ʈ ���� : IP �ּ� = {}, ��Ʈ ��ȣ = {}\n", addr, ntohs(clientaddr.sin_port));
	ioLock.unlock();

	cmd = make_unique<Command>(CMD_TYPE::LOGOUT);
	cmd->context = make_shared<CMD_Logout>(client_sock);
	excuteLock.lock();
	excuteQueue.emplace(move(cmd));
	cv.notify_all();
	excuteLock.unlock();
}

void Server::Excute()
{
	while (true) {
		unique_lock<mutex> queueLock(mutexes[EXCUTE]);
		cv.wait(queueLock, [&] {return !excuteQueue.empty(); });
		auto cmd = move(excuteQueue.front());
		excuteQueue.pop();
		queueLock.unlock();
		
		switch (cmd->type)
		{
		case CMD_TYPE::LOGIN_SUCCESS:
		{
			auto context = static_pointer_cast<CMD_LoginSuccess>(cmd->context);
			auto player = make_unique<Player>();
			player->id = context->appendSock;
			player->color = Random::RandInt(0, colors.size() - 1);
			//sf::Vector2f pos(Random::RandInt(Player::startSize, PlayScene::worldWidth - Player::startSize), Random::RandInt(Player::startSize, PlayScene::worldHeight - Player::startSize));
			sf::Vector2f pos(Random::RandInt(100, 150), Random::RandInt(100, 150));

			player->SetPosition(pos);
			// ���߿� �α��� �� ���� �̸����� ������ ����
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


			auto newCMD = make_unique<Command>(CMD_TYPE::PLAYER_APPEND);
			auto newContext = make_shared<CMD_PlayerAppend>(context->appendSock);
			newCMD->context = newContext;

			queueLock.lock();
			excuteQueue.emplace(move(newCMD));
			queueLock.unlock();
		}
		break;
		case CMD_TYPE::PLAYER_APPEND:
		{
			auto context = static_pointer_cast<CMD_PlayerAppend>(cmd->context);
			auto packet = make_shared<PlayerAppend>();
			unique_lock<mutex> entityLock(mutexes[ENTITIES]);
			auto iter = find_if(players.begin(), players.end(), [&](const auto& player) {return player->id == context->appendSock; });
			if (iter != players.end()) {
				const auto& player = *iter;
				packet->color = player->color;
				packet->id = player->id;
				packet->x = player->Position().x;
				packet->y = player->Position().y;
				memcpy(packet->name, player->name, 16);
				entityLock.unlock();
				uint type = htonl(PACKET_TYPE::PLAYER_APPEND);
				for (auto sock : clients) {
					if (sock != context->appendSock) {
						send(sock, (char*)&type, sizeof(uint), 0);
						packet->Send(sock);
					}
				}
			}
			else {
				entityLock.unlock();
			}
		}
		break;
		case CMD_TYPE::PLAYER_INPUT:
		{
			auto context = static_pointer_cast<CMD_PlayerInput>(cmd->context);
			unique_lock<mutex> lock(mutexes[ENTITIES]);
			auto iter = find_if(players.begin(), players.end(), [&](const auto& player) {return player->id == context->id; });
			if (iter != players.end()) {
				const auto& player = *iter;
				player->SetDestination(player->Position().x + context->x, player->Position().y + context->y);
				auto packet = make_shared<PlayerInput>(player->id, player->Destination().x, player->Destination().y);
				uint type = htonl(PACKET_TYPE::PLAYER_INPUT);
				for (auto sock : clients) {
					send(sock, (char*)&type, sizeof(uint), 0);
					packet->Send(sock);
				}
				lock.unlock();

			}
			else {
				lock.unlock();
			}
		}
		break;
		case CMD_TYPE::LOGOUT:
		{
			auto context = static_pointer_cast<CMD_Logout>(cmd->context);
			auto payload = make_shared<Logout>();
			payload->id = context->outSock;
			uint type = htonl(PACKET_TYPE::LOGOUT);
			unique_lock<mutex> clientLock(mutexes[CLIENT_SOCK]);
			erase(clients, context->outSock);
			for (auto& sock : clients) {
				send(sock, (char*)&type, sizeof(uint), 0);
				payload->Send(sock);
			}
			closesocket(context->outSock);
			clientLock.unlock();

			lock_guard<mutex> entityLock(mutexes[ENTITIES]);
			erase_if(players, [&](auto& player) {return player->id == (int)context->outSock; });
		}
		break;
		case CMD_TYPE::CHECK_COLLISION:
		{
			auto context = static_pointer_cast<CMD_CheckCollision>(cmd->context);

			unique_lock<mutex> lock(mutexes[ENTITIES]);
			auto iter1 = find_if(players.begin(), players.end(), [&](const auto& player) { return player->id == context->id1; });
			auto iter2 = find_if(players.begin(), players.end(), [&](const auto& player) { return player->id == context->id2; });

			if (iter1 != players.end() && iter2 != players.end()) {
				auto& player1 = *iter1;
				auto& player2 = *iter2;

				// �浹 ó�� ����: ���� ������ ����
				if (player1->size > player2->size) {
					player1->size += player2->size * 0.5f; // �й����� �Ϻ� ũ�⸦ ���ڰ� ���
					players.erase(iter2);                 // �й� �÷��̾� ����
				}
				else {
					player2->size += player1->size * 0.5f;
					players.erase(iter1);
				}

				// ��� Ŭ���̾�Ʈ�� �浹 ������ �˸�
				auto packet = make_shared<ConfirmCollision>();
				packet->id1 = context->id1;
				packet->id2 = context->id2;
				uint type = htonl(PACKET_TYPE::CHECK_COLLISION);

				lock.unlock(); // ��� ���� �� Ŭ���̾�Ʈ ��� ó��
				unique_lock<mutex> clientLock(mutexes[CLIENT_SOCK]);
				for (auto& sock : clients) {
					send(sock, (char*)&type, sizeof(uint), 0);
					packet->Send(sock);
				}
			}
		}
		break;


		default:
			break;
		}
	}
}
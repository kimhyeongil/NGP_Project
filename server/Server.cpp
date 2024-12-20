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
	foods.resize(1000);
	for (auto& food : foods) {
		static int id = -1;
		food = make_unique<Food>(id--);
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
		uint type;
		if (recv(client_sock, (char*)&type, sizeof(uint), MSG_WAITALL) == SOCKET_ERROR) {
				break;
		}
		type = ntohl(type);
		if(type==PACKET_TYPE::PLAYER_NAME){
		auto input = make_shared<PlayerName>();
			input->Recv(client_sock);
			
			print("[TCP 서버] 클라이언트 입력:포트 번호 = {}, 이름 ({})\n", ntohs(clientaddr.sin_port), input->name);
		
			auto cmd = make_unique<Command>(CMD_TYPE::LOGIN_SUCCESS);
			cmd->context = make_shared<CMD_LoginSuccess>(client_sock,input->name);
			lock_guard<mutex> excuteLock(mutexes[EXCUTE]);
			excuteQueue.emplace(move(cmd));
			cv.notify_one();
		}
	}
}

void Server::Run()
{
	auto start = chrono::high_resolution_clock::now();
	double broadcastTime = 0.0;
	while (true)
	{
		auto end = chrono::high_resolution_clock::now();
		double deltaTime = chrono::duration_cast<chrono::nanoseconds>(end - start).count() * 1e-9;
		if (deltaTime < 1e-4) {
			this_thread::sleep_for(100ns);
			end = chrono::high_resolution_clock::now();
			deltaTime = chrono::duration_cast<chrono::nanoseconds>(end - start).count() * 1e-9;
		}
		recreateDeltaTime += deltaTime;
		broadcastTime += deltaTime;
		if (recreateDeltaTime >= recreateTime) {
			recreateDeltaTime -= recreateTime;
			lock_guard<mutex> lock(mutexes[EXCUTE]);
			excuteQueue.emplace(make_unique<Command>(CMD_TYPE::RECREATE_FOOD));
			cv.notify_one();
		}
		if (broadcastTime >= 1.0) {
			broadcastTime -= 1.0;
			lock_guard<mutex> lock(mutexes[EXCUTE]);
			excuteQueue.emplace(make_unique<Command>(CMD_TYPE::BROADCAST));
			cv.notify_one();
		}
		start = end;
		Update(deltaTime);
		CheckCollision();
	}
}

void Server::CheckCollision()
{
	lock_guard<mutex> entityLock(mutexes[ENTITIES]);

	for (auto it1 = players.begin(); it1 != players.end(); ++it1) {
		auto& player1 = *it1;
		if (!player1->active) {
			continue;
		}
		for (auto it2 = next(it1); it2 != players.end(); ++it2) {
			auto& player2 = *it2;
			if (!player2->active) {
				continue;
			}
			float distance = sf::Vector2f::Distance(player1->Position(), player2->Position());

			float combinedRadius = player1->Radius() + player2->Radius();
			
			if (distance < combinedRadius) {
				player1->OnCollision(player2.get());
				player2->OnCollision(player1.get());

				auto cmd = make_unique<Command>(CMD_TYPE::CHECK_COLLISION);
				auto context = make_shared<CMD_CheckCollision>(player1->id, player2->id);
				cmd->context = context;

				lock_guard<mutex> executeLock(mutexes[EXCUTE]);
				excuteQueue.emplace(move(cmd));
			}
		}
	}
	for (const auto& player : players) {
		if (!player->active) {
			continue;
		}
		for (const auto& food : foods) {
			if (!food->active) {
				continue;
			}
			float distance = sf::Vector2f::Distance(player->Position(), food->Position());
			float combinedRadius = player->Radius() + food->Radius();
			if (distance < combinedRadius) {
				player->OnCollision(food.get());
				food->OnCollision(player.get());

				auto cmd = make_unique<Command>(CMD_TYPE::CHECK_COLLISION);
				auto context = make_shared<CMD_CheckCollision>(player->id, food->id);
				cmd->context = context;

				lock_guard<mutex> executeLock(mutexes[EXCUTE]);
				excuteQueue.emplace(move(cmd));
			}
		}
	}
	cv.notify_one();
}

void Server::Update(double deltaTime)
{
	lock_guard<mutex> entityLock(mutexes[ENTITIES]);
	for (auto& player : players) {
		player->Update(deltaTime);
	}
	for (auto& food : foods) {
		food->Update(deltaTime);
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
	print("[TCP 서버] 클라이언트 접속 : IP 주소 = {}, 포트 번호 = {}\n", addr, ntohs(clientaddr.sin_port));
	ioLock.unlock();

	uint type;
	while (true) {
		if (recv(client_sock, (char*)&type, sizeof(uint), MSG_WAITALL) == SOCKET_ERROR) {
			break;
		}
		type = ntohl(type);

		switch (type) {
		case PACKET_TYPE::PLAYER_INPUT: 
		{
			auto input = make_shared<PlayerInput>();
			input->Recv(client_sock);
			ioLock.lock();
			print("[TCP 서버] 클라이언트 입력: IP 주소 = {}, 포트 번호 = {}, 위치 ({}, {})\n", addr, ntohs(clientaddr.sin_port), input->x, input->y);
			ioLock.unlock();
			auto cmd = make_unique<Command>(CMD_TYPE::PLAYER_INPUT);
			auto context = make_shared<CMD_PlayerInput>();
			context->id = input->id;
			context->x = input->x;
			context->y = input->y;
			cmd->context = context;

			lock_guard<mutex> excuteLock(mutexes[EXCUTE]);
			excuteQueue.emplace(move(cmd));
			cv.notify_one();
		}
		break;
		case PACKET_TYPE::RESTART_TO_SERVER: 
		{
			auto input = make_shared<RestartToServer>();
			input->Recv(client_sock);

			auto cmd = make_unique<Command>(CMD_TYPE::PLAYER_RESTART);
			cmd->context = make_shared<CMD_Restart>(input->id);

			lock_guard<mutex> excuteLock(mutexes[EXCUTE]);
			excuteQueue.emplace(move(cmd));
			cv.notify_one();
		}
		break;
		default:
			break;
		}
	}

	ioLock.lock();
	print("[TCP 서버] 클라이언트 종료 : IP 주소 = {}, 포트 번호 = {}\n", addr, ntohs(clientaddr.sin_port));
	ioLock.unlock();

	auto cmd = make_unique<Command>(CMD_TYPE::LOGOUT);
	cmd->context = make_shared<CMD_Logout>(client_sock);

	lock_guard<mutex> excuteLock(mutexes[EXCUTE]);
	excuteQueue.emplace(move(cmd));
	cv.notify_one();
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
			clients.emplace_back(context->appendSock);
			thread(&Server::ProcessClient, this, context->appendSock).detach();

			auto player = make_unique<Player>(context->appendSock);
			player->SetPosition(Random::RandInt(Player::startSize, PlayScene::worldWidth - Player::startSize), 
				Random::RandInt(Player::startSize, PlayScene::worldWidth - Player::startSize));
			player->color = Random::RandInt(0, colors.size() - 1);
			memcpy(player->name, context->name, sizeof(player->name));

			vector<PlayerInfo> infos;
			infos.emplace_back(*player);

			unique_lock<mutex> entityLock(mutexes[ENTITIES]);
			for (const auto& p : players) {
				infos.emplace_back(*p);
			}
			auto newCMD = make_unique<Command>(CMD_TYPE::PLAYER_APPEND);
			auto newContext = make_shared<CMD_PlayerAppend>(context->appendSock, player->Position().x, player->Position().y, player->color, context->name);
			memcpy(newContext->name, context->name, 16);
			newCMD->context = newContext;

			queueLock.lock();
			excuteQueue.emplace(move(newCMD));
			queueLock.unlock();

			players.emplace_front(move(player));
			vector<FoodInfo> foodInfos;
			foodInfos.reserve(foods.size());
			for (const auto& f : foods) {
				foodInfos.emplace_back(*f);
			}
			entityLock.unlock();
			LoginSuccess packet;
			packet.players = infos;
			packet.foods = foodInfos;

			uint type = (uint)PACKET_TYPE::LOGIN_SUCCESS;
			type = htonl(type);
			send(context->appendSock, (char*)&type, sizeof(uint), 0);
			packet.Send(context->appendSock);
		}
		break;
		case CMD_TYPE::BROADCAST:
		{
			unique_lock<mutex> entityLock(mutexes[ENTITIES]);
			vector<PlayerInfo> infos;
			for (const auto& player : players) {
				infos.emplace_back(*player);
			}
			entityLock.unlock();

			BroadCast packet;
			packet.players = infos;

			uint type = htonl(PACKET_TYPE::BROADCAST);
			for (auto sock : clients) {
				send(sock, (char*)&type, sizeof(uint), 0);
				packet.Send(sock);
			}
		}
		break;
		case CMD_TYPE::PLAYER_APPEND:
		{
			auto context = static_pointer_cast<CMD_PlayerAppend>(cmd->context);
			PlayerAppend packet;

			packet.color = context->color;
			packet.id = context->appendSock;
			packet.x = context->x;
			packet.y = context->y;
			memcpy(packet.name, context->name, 16);

			uint type = htonl(PACKET_TYPE::PLAYER_APPEND);
			for (auto sock : clients) {
				if (sock != context->appendSock) {
					send(sock, (char*)&type, sizeof(uint), 0);
					packet.Send(sock);
				}
			}
		}
		break;
		case CMD_TYPE::PLAYER_INPUT:
		{
			auto context = static_pointer_cast<CMD_PlayerInput>(cmd->context);
			lock_guard<mutex> lock(mutexes[ENTITIES]);
			auto iter = find_if(players.begin(), players.end(), [&](const auto& player) {return player->id == context->id; });

			if (iter != players.end()) {
				const auto& player = *iter;
				player->SetDestination(player->Position().x + context->x, player->Position().y + context->y);

				PlayerInput packet{ player->id, player->Destination().x, player->Destination().y };
				uint type = htonl(PACKET_TYPE::PLAYER_INPUT);
				for (auto sock : clients) {
					send(sock, (char*)&type, sizeof(uint), 0);
					packet.Send(sock);
				}
			}
		}
		break;
		case CMD_TYPE::LOGOUT:
		{
			auto context = static_pointer_cast<CMD_Logout>(cmd->context);
			Logout packet;
			packet.id = context->outSock;

			uint type = htonl(PACKET_TYPE::LOGOUT);
			erase(clients, context->outSock);
			for (auto& sock : clients) {
				send(sock, (char*)&type, sizeof(uint), 0);
				packet.Send(sock);
			}
			lock_guard<mutex> entityLock(mutexes[ENTITIES]);
			erase_if(players, [&](auto& player) {return player->id == (int)context->outSock; });

			closesocket(context->outSock);
		}
		break;
		case CMD_TYPE::CHECK_COLLISION:
		{
			auto context = static_pointer_cast<CMD_CheckCollision>(cmd->context);

			// 모든 클라이언트에 충돌 정보를 알림
			ConfirmCollision packet;
			packet.id1 = context->id1;
			packet.id2 = context->id2;

			uint type = htonl(PACKET_TYPE::CHECK_COLLISION);
			for (auto& sock : clients) {
				send(sock, (char*)&type, sizeof(uint), 0);
				packet.Send(sock);
			}
		}
		break;
		case CMD_TYPE::RECREATE_FOOD:
		{
			int i = 0;
			vector<FoodInfo> data;
			data.reserve(maxReCnt);
			unique_lock<mutex> lock(mutexes[ENTITIES]);
			for (const auto& food : foods) {
				if (!food->active) {
					i++;
					food->Reset();
					data.emplace_back(*food);
				}
				if (i >= maxReCnt) {
					break;
				}
			}
			lock.unlock();
			RecreateFood packet;
			packet.foods = data;

			uint type = htonl(PACKET_TYPE::RECREATE_FOOD);
			for (auto& sock : clients) {
				send(sock, (char*)&type, sizeof(uint), 0);
				packet.Send(sock);
			}
		}
		break;
		case CMD_TYPE::PLAYER_RESTART:
		{
			auto context = static_pointer_cast<CMD_Restart>(cmd->context);

			unique_lock<mutex> lock(mutexes[ENTITIES]);
			auto iter = find_if(players.begin(), players.end(), [&](const auto& e) {return e->id == context->id; });
			if (iter != players.end()) {
				const auto& player = *iter;
				player->SetPosition(Random::RandInt(player->startSize, PlayScene::worldWidth - player->startSize)
								   ,Random::RandInt(player->startSize, PlayScene::worldWidth - player->startSize));
				player->active = true;
				player->color = Random::RandInt(0, colors.size() - 1);

				RestartToClient context;
				context.color = player->color;
				context.x = player->Position().x;
				context.y = player->Position().y;
				context.id = player->id;
				lock.unlock();

				uint type = PACKET_TYPE::RESTART_TO_CLIENT;
				type = htonl(type);

				for (const auto& client : clients) {
					send(client, (char*)&type, sizeof(uint), 0);
					context.Send(client);
				}
			}
		}
		break;
		default:
			break;
		}
	}
}

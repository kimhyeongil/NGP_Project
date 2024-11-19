#define _CRT_SECURE_NO_WARNINGS // Disable warnings for old C functions
//#define _WINSOCK_DEPRECATED_NO_WARNINGS // Disable warnings for old socket API
#include <thread>
#include <winsock2.h> // Winsock2 main header
#include <ws2tcpip.h> // Winsock2 extension header
#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <deque>
#include <mutex>
#include <vector>
#include "../Client/Entity.h"
#include "..//Common.h"
#include "../server/Command.h"
#include "../SFML/include/SFML/System.hpp"

#pragma comment(lib, "ws2_32") // Link with ws2_32.lib


extern std::vector<ClientInfo> clientInfos;
extern std::deque<QueueBasket> requestQueue;
extern std::vector<Player> players;
extern std::mutex clientCountMutex;
extern std::mutex queueMutex;
extern std::mutex clientMutex;
extern std::mutex playerMutex;

extern int client_count;
void processClient(SOCKET client_sock);
uint hton_requestType(uint type);
uint ntoh_requestType(uint type);
// ��� Ŭ���̾�Ʈ���� �����͸� �����ϴ� �Լ�
void sendToAllClients(const char* data, int dataSize);
// �÷��̾� ������ ��� Ŭ���̾�Ʈ���� ����
void sendPlayerInfoToAllClients();
void sendAllPlayerInfos(SOCKET clientSocket);
void sendToEveryoneElse(SOCKET exceptSocket, const char* data, int dataSize);
void sendNewPlayerInfoToAllClients(SOCKET exceptSocket, const Player& newPlayer);
// Ŭ���̾�Ʈ�� ��Ͽ��� �����ϰ� �ٸ� Ŭ���̾�Ʈ���� �˸�
void removeClient(SOCKET clientSocket);
// Ŭ���̾�Ʈ ��û ó�� ������
void execute();
// Ŭ���̾�Ʈ ������ �Լ�
void processClient(SOCKET client_sock);
// ���� �Լ� ���� ���
void err_display(const char* msg);

// ���� �Լ� ���� ���
void err_display(int errcode);

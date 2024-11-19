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
// 모든 클라이언트에게 데이터를 전송하는 함수
void sendToAllClients(const char* data, int dataSize);
// 플레이어 정보를 모든 클라이언트에게 전송
void sendPlayerInfoToAllClients();
void sendAllPlayerInfos(SOCKET clientSocket);
void sendToEveryoneElse(SOCKET exceptSocket, const char* data, int dataSize);
void sendNewPlayerInfoToAllClients(SOCKET exceptSocket, const Player& newPlayer);
// 클라이언트를 목록에서 삭제하고 다른 클라이언트에게 알림
void removeClient(SOCKET clientSocket);
// 클라이언트 요청 처리 쓰레드
void execute();
// 클라이언트 쓰레드 함수
void processClient(SOCKET client_sock);
// 소켓 함수 오류 출력
void err_display(const char* msg);

// 소켓 함수 오류 출력
void err_display(int errcode);

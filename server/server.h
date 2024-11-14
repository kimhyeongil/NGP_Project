#define _CRT_SECURE_NO_WARNINGS // Disable warnings for old C functions
//#define _WINSOCK_DEPRECATED_NO_WARNINGS // Disable warnings for old socket API

#include <winsock2.h> // Winsock2 main header
#include <ws2tcpip.h> // Winsock2 extension header
#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <deque>
#include "Entity.h"
#include "..//Common.h"

#pragma comment(lib, "ws2_32") // Link with ws2_32.lib

struct PlayerInfo {
	int id;
	int color;
	float posX;
	float posY;
};

struct ClientInfo {
	int id;
	SOCKET clientSocket;
};

struct QueueBasket {
	uint requestType = LOGIN_TRY;
	SOCKET clientSocket;
	char clientName[16];
};

extern std::vector<ClientInfo> clientInfos;
extern std::deque<QueueBasket> requestQueue;
extern std::vector<Player> players;
// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

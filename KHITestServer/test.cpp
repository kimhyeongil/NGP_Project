#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더
#include <iostream>
#include <string>
#include <memory>
#include <print>
#include <filesystem>
#include "../Common.h"
#pragma comment(lib, "ws2_32") // ws2_32.lib 링크

using namespace std;
namespace fs = std::filesystem;

#define SERVERPORT 9000
#define BUFSIZE    128

#undef min

CRITICAL_SECTION cs;

string currDir;
int line = 0;

DWORD WINAPI ProcessClient(void* arg)
{
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;

	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	EnterCriticalSection(&cs);
	int row = line++;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD(0, row));
	std::print("[TCP 서버] 클라이언트 접속 : IP 주소 = {}, 포트 번호 = {}", addr, ntohs(clientaddr.sin_port));
	LeaveCriticalSection(&cs);

	int retval;
	uint type;

	while (1) {

		if (recv(client_sock, (char*)&type, sizeof(uint), MSG_WAITALL) == SOCKET_ERROR) {
			break;
		}

		type = ntohl(type);
		if (type == PACKET_TYPE::PLAYER_INPUT) {
			PlayerInput input;
			if (recv(client_sock, (char*)&input, sizeof(PlayerInput), MSG_WAITALL) == SOCKET_ERROR) {
				break;
			}
			input.ntoh();

			EnterCriticalSection(&cs);
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD(0, line++));
			print("[TCP 서버] 클라이언트 입력: IP 주소 = {}, 포트 번호 = {}, 위치 ({}, {})", addr, ntohs(clientaddr.sin_port), input.x, input.y);
			LeaveCriticalSection(&cs);
		}
	}

	closesocket(client_sock);
	EnterCriticalSection(&cs);
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD(0, line++));
	print("[TCP 서버] 클라이언트 종료 : IP 주소 = {}, 포트 번호 = {}", addr, ntohs(clientaddr.sin_port));
	LeaveCriticalSection(&cs);

	return 0;
}

int main(int argc, char* argv[])
{
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (listen_sock == INVALID_SOCKET) {
		WSACleanup();
		return -1;
	}
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);

	int retval;
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR) {
		closesocket(listen_sock);
		WSACleanup();
		return -1;
	}

	retval = listen(listen_sock, SOMAXCONN);

	if (retval == SOCKET_ERROR) {
		closesocket(listen_sock);
		WSACleanup();
		return -1;
	}

	currDir = fs::current_path().string();

	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;

	HANDLE hThread;
	InitializeCriticalSection(&cs);
	while (1) {
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			break;
		}

		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);

		if (hThread == NULL) { closesocket(client_sock); }
		else { CloseHandle(hThread); }
	}

	DeleteCriticalSection(&cs);
	closesocket(listen_sock);
	WSACleanup();

	return 0;
}
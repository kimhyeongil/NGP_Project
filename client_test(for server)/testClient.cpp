#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "..//Common.h"

#pragma comment(lib, "ws2_32")
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFSIZE 512

uint hton_requestType(uint type) {
    return static_cast<uint>(htonl(static_cast<int>(type)));
}

uint ntoh_requestType(uint type) {
    return static_cast<uint>(ntohl(static_cast<int>(type)));
}
struct PlayerInfo {
    int id;
    int color;
    float posX;
    float posY;
};
int main() {
    WSADATA wsa;
    SOCKET sock;
    sockaddr_in server_addr;
    char buf[BUFSIZE];
    int retval;

    // Winsock 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup() failed.\n";
        return -1;
    }

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed.\n";
        WSACleanup();
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);

    // 서버에 연결
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "connect() failed.\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // LOGIN_TRY 요청 타입 전송
    uint requestType = hton_requestType(LOGIN_TRY);
    retval = send(sock, reinterpret_cast<char*>(&requestType), static_cast<int>(sizeof(requestType)), 0);
    if (retval == SOCKET_ERROR) {
        std::cerr << "Failed to send LOGIN_TRY request.\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    while (true) {
        // 클라이언트 이름 전송
        std::string clientName = "Player1";
        strncpy_s(buf, clientName.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        retval = send(sock, buf, static_cast<int>(strlen(buf) + 1), 0);
        if (retval == SOCKET_ERROR) {
            std::cerr << "Failed to send client name.\n";
            closesocket(sock);
            WSACleanup();
            return -1;
        }

        // 서버로부터 LOGIN_SUCCESS 응답 수신
        retval = recv(sock, buf, sizeof(uint), 0); // 패킷 타입 수신
        if (retval <= 0) {
            std::cerr << "Failed to receive response from server.\n";
            closesocket(sock);
            WSACleanup();
            return -1;
        }
        uint responseType;
        memcpy(&responseType, buf, sizeof(uint));
        responseType = ntoh_requestType(responseType);

        if (responseType == LOGIN_SUCCESS) {
            // 플레이어 수 수신
            //retval = recv(sock, buf, sizeof(uint), 0);
            //if (retval <= 0) {
            //    std::cerr << "Failed to receive player count.\n";
            //    closesocket(sock);
            //    WSACleanup();
            //    return -1;
            //}
            PlayerCount cnt;
            cnt.Recv(sock);
            //memcpy(&playerCount, buf, sizeof(uint));
            //playerCount = ntohl(playerCount);

            std::cout << "Player Count: " << cnt.cnt << std::endl;

            // 플레이어 정보 수신
            for (uint i = 0; i < cnt.cnt; ++i) {
                retval = recv(sock, buf, sizeof(LoginSuccess), 0);
                if (retval <= 0) {
                    std::cerr << "Failed to receive player info.\n";
                    closesocket(sock);
                    WSACleanup();
                    return -1;
                }

                LoginSuccess playerInfo;
                memcpy(&playerInfo, buf, sizeof(LoginSuccess));
                playerInfo.ntoh();
                std::cout << "Player ID: " << playerInfo.id << ", Color: " << playerInfo.color
                    << ", Position: (" << playerInfo.x << ", " << playerInfo.y << ")\n";
            }
        }
        if (responseType == PLAYER_APPEND) {
            // 플레이어 수 수신


            

            PlayerAppend playerInfo;
            playerInfo.Recv(sock);
            memcpy(&playerInfo, buf, sizeof(PlayerAppend));
            playerInfo.ntoh();
            std::cout << "Player ID: " << playerInfo.id << ", Color: " << playerInfo.color
                << ", Position: (" << playerInfo.x << ", " << playerInfo.y << ")\n";

        }
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}
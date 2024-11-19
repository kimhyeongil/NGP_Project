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

    // Winsock �ʱ�ȭ
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup() failed.\n";
        return -1;
    }

    // ���� ����
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket() failed.\n";
        WSACleanup();
        return -1;
    }

    // ���� �ּ� ����
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    server_addr.sin_port = htons(SERVER_PORT);

    // ������ ����
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "connect() failed.\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // LOGIN_TRY ��û Ÿ�� ����
    uint requestType = hton_requestType(LOGIN_TRY);
    retval = send(sock, reinterpret_cast<char*>(&requestType), static_cast<int>(sizeof(requestType)), 0);
    if (retval == SOCKET_ERROR) {
        std::cerr << "Failed to send LOGIN_TRY request.\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    while (true) {
        // Ŭ���̾�Ʈ �̸� ����
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

        // �����κ��� LOGIN_SUCCESS ���� ����
        retval = recv(sock, buf, sizeof(uint), 0); // ��Ŷ Ÿ�� ����
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
            // �÷��̾� �� ����
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

            // �÷��̾� ���� ����
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
            // �÷��̾� �� ����


            

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
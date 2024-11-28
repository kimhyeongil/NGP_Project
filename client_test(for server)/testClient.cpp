#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include "../Common.h"


#pragma comment(lib, "ws2_32.lib")

void handleServerMessages(SOCKET sock) {
    while (true) {
        uint packetType;
        if (recv(sock, (char*)&packetType, sizeof(uint), MSG_WAITALL) <= 0) {
            std::cerr << "Server disconnected or error occurred.\n";
            break;
        }

        packetType = ntohl(packetType);

        switch (packetType) {
        case PACKET_TYPE::BROADCAST: {
            BroadCast broadcastPacket;
            broadcastPacket.Recv(sock);

            std::cout << "Received BROADCAST:\n";
            for (const auto& player : broadcastPacket.players) {
                std::cout << "Player ID: " << player.id
                    << ", Position: (" << player.x << ", " << player.y << ")"
                    << ", Color: " << player.color << "\n";
            }
            break;
        }
        default:
           // std::cout << "Unknown packet type received: " << packetType << "\n";
            break;
        }

    }
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initialize Winsock.\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    // Use inet_pton instead of inet_addr
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address format.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    serverAddr.sin_port = htons(9000);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server.\n";

    // 서버 메시지 수신용 스레드
    std::thread listener(handleServerMessages, sock);
    listener.detach();

    // 사용자 입력 시뮬레이션 (종료 명령 전까지 유지)
    while (true) {
        std::string input;
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        std::cout << "Type 'exit' to quit.\n";
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

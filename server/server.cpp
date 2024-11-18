#include "server.h"
#include <thread>
#include <string>
#include <cstring>
#include <algorithm>
#include <SFML/System.hpp>
#include <vector>
#include <deque>
#include <mutex>

#define SERVERPORT 9000
#define BUFSIZE 512

std::vector<ClientInfo> clientInfos;
std::deque<QueueBasket> requestQueue;
std::vector<Player> players;
std::mutex clientCountMutex;
std::mutex queueMutex;
std::mutex clientMutex;
std::mutex playerMutex;
int client_count = 0;

uint hton_requestType(uint type) {
    return static_cast<uint>(htonl(static_cast<int>(type)));
}

uint ntoh_requestType(uint type) {
    return static_cast<uint>(ntohl(static_cast<int>(type)));
}

// ClientInfo 변환 함수 (네트워크 바이트 정렬)
void hton_clientInfo(ClientInfo& client) {
    client.id = htonl(client.id);
}

void ntoh_clientInfo(ClientInfo& client) {
    client.id = ntohl(client.id);
}

// 모든 클라이언트에게 데이터를 전송하는 함수
void sendToAllClients(const char* data, int dataSize) {
    std::lock_guard<std::mutex> lock(queueMutex);
    for (const auto& client : clientInfos) {
        send(client.clientSocket, data, dataSize, 0);
    }
}

// 플레이어 정보를 모든 클라이언트에게 전송
void sendPlayerInfoToAllClients() {
    uint packetType = hton_requestType(LOGIN_SUCCESS);
    sendToAllClients(reinterpret_cast<char*>(&packetType), sizeof(packetType));

    uint playerCount = htonl(players.size());
    sendToAllClients(reinterpret_cast<char*>(&playerCount), sizeof(playerCount));

    std::vector<char> allPlayerData;
    for (const auto& player : players) {
        PlayerInfo playerInfo;
        playerInfo.id = htonl(player.id);
        playerInfo.color = htonl(player.color);
        playerInfo.posX = htonl(player.Position().x);
        playerInfo.posY = htonl(player.Position().y);

        allPlayerData.insert(allPlayerData.end(), reinterpret_cast<char*>(&playerInfo), reinterpret_cast<char*>(&playerInfo) + sizeof(PlayerInfo));
    }

    sendToAllClients(allPlayerData.data(), allPlayerData.size());
}

// 클라이언트를 목록에서 삭제하고 다른 클라이언트에게 알림
void removeClient(SOCKET clientSocket) {
    std::lock_guard<std::mutex> lock(clientMutex);
    auto clientIt = std::find_if(clientInfos.begin(), clientInfos.end(), [&](const ClientInfo& client) {
        return client.clientSocket == clientSocket;
        });

    if (clientIt != clientInfos.end()) {
        int clientId = clientIt->id;
        clientInfos.erase(clientIt);

        std::lock_guard<std::mutex> playerLock(playerMutex);
        auto playerIt = std::find_if(players.begin(), players.end(), [&](const Player& player) {
            return player.id == clientId;
            });

        if (playerIt != players.end()) {
            players.erase(playerIt);
        }
    }
}

// 클라이언트 요청 처리 쓰레드
void execute() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (!requestQueue.empty()) {
            QueueBasket req = requestQueue.front();
            requestQueue.pop_front();
            lock.unlock();

            if (req.requestType == LOGIN_TRY) {
                ClientInfo newClient;
                {
                    std::lock_guard<std::mutex> countLock(clientCountMutex);
                    newClient.id = client_count++;
                }
                newClient.clientSocket = req.clientSocket;

                Player newPlayer;
                newPlayer.id = newClient.id;
                newPlayer.shape.setPosition(rand() % 800, rand() % 600);
                int randomIndex = std::rand() % colors.size();
                newPlayer.color = randomIndex;

                {
                    std::lock_guard<std::mutex> clientLock(clientMutex);
                    clientInfos.push_back(newClient);
                }
                {
                    std::lock_guard<std::mutex> playerLock(playerMutex);
                    players.push_back(newPlayer);
                }

                //uint successMsg = LOGIN_SUCCESS;
                //successMsg = hton_requestType(successMsg);
                printf("Client [%s] 로그인 성공 (LOGIN_SUCCESS 전송)\n", req.clientName);

                sendPlayerInfoToAllClients();
            }
        }
        else {
            lock.unlock();
        }
    }
}

// 클라이언트 쓰레드 함수
void processClient(SOCKET client_sock) {
    char buf[BUFSIZE];
    int retval;
    while (true) {
        retval = recv(client_sock, buf, sizeof(uint), 0);
        if (retval <= 0) {
            removeClient(client_sock);
            closesocket(client_sock);
            return;
        }

        uint requestType;
        memcpy(&requestType, buf, sizeof(uint));
        requestType = ntoh_requestType(requestType);
        if (requestType == LOGIN_TRY) {
            retval = recv(client_sock, buf, sizeof(buf), 0);
            if (retval <= 0) {
                removeClient(client_sock);
                closesocket(client_sock);
                printf("못받았음 이름\n");
                return;
            }

            QueueBasket req;
            req.requestType = LOGIN_TRY;
            req.clientSocket = client_sock;
            strncpy(req.clientName, buf, sizeof(req.clientName) - 1);
            req.clientName[sizeof(req.clientName) - 1] = '\0';

            std::lock_guard<std::mutex> lock(queueMutex);
            requestQueue.push_back(req);
            printf("리시브 받고 넘겨줌 %d\n", requestType);
        }
    }
}

int main(int argc, char* argv[]) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    listen(listen_sock, SOMAXCONN);

    std::thread executeThread(execute);
    executeThread.detach();

    while (true) {
        SOCKET client_sock = accept(listen_sock, nullptr, nullptr);
        std::thread clientThread(processClient, client_sock);
        clientThread.detach();
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}

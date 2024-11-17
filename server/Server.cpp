#include "server.h"
#include <thread>
#include <string>
#include <cstring>
#include <algorithm>  // ������ ���� �߰�
#include <SFML/System.hpp> // SFML ���� ����� ���� ���
#define SERVERPORT 9000
#define BUFSIZE 512

std::vector<ClientInfo> clientInfos;
std::deque<QueueBasket> requestQueue;
std::vector<Player> players;
CRITICAL_SECTION csClientCount;
CRITICAL_SECTION csQueue;   // ť ���� �Ӱ� ����
CRITICAL_SECTION csClient;  // Ŭ���̾�Ʈ ��� ���� �Ӱ� ����
CRITICAL_SECTION csPlayer;  // �÷��̾� ��� ���� �Ӱ� ����
int client_count = 0;

uint hton_requestType(uint type) {
    return static_cast<uint>(htonl(static_cast<int>(type)));
}

uint ntoh_requestType(uint type) {
    return static_cast<uint>(ntohl(static_cast<int>(type)));
}

// ClientInfo ��ȯ �Լ� (��Ʈ��ũ ����Ʈ ����)
void hton_clientInfo(ClientInfo& client) {
    client.id = htonl(client.id);
}

void ntoh_clientInfo(ClientInfo& client) {
    client.id = ntohl(client.id);
}

// ��� Ŭ���̾�Ʈ���� �����͸� �����ϴ� �Լ�
void sendToAllClients(const char* data, int dataSize) {
    EnterCriticalSection(&csQueue);
    for (const auto& client : clientInfos) {
        send(client.clientSocket, data, dataSize, 0);
    }
    LeaveCriticalSection(&csQueue);
}

// �÷��̾� ������ ��� Ŭ���̾�Ʈ���� ����
void sendPlayerInfoToAllClients() {
    // ��Ŷ Ÿ�԰� ũ�⸦ ����
    uint packetType = hton_requestType(LOGIN_SUCCESS);
    sendToAllClients(reinterpret_cast<char*>(&packetType), sizeof(packetType));

    // �÷��̾� ���� ����
    uint playerCount = htonl(players.size());
    sendToAllClients(reinterpret_cast<char*>(&playerCount), sizeof(playerCount));

    // �÷��̾� �������� �ϳ��� ū ���ۿ� ��Ƽ� ����
    std::vector<char> allPlayerData;
    for (const auto& player : players) {
        PlayerInfo playerInfo;
        playerInfo.id = htonl(player.id);
        playerInfo.color = htonl(player.color);
        playerInfo.posX = player.Position().x;
        playerInfo.posY = player.Position().y;

        // �� PlayerInfo�� ū ���ۿ� �߰�
        allPlayerData.insert(allPlayerData.end(), reinterpret_cast<char*>(&playerInfo), reinterpret_cast<char*>(&playerInfo) + sizeof(PlayerInfo));
    }

    // ��� �÷��̾� �����͸� �� ���� ����
    sendToAllClients(allPlayerData.data(), allPlayerData.size());
}

// Ŭ���̾�Ʈ�� ��Ͽ��� �����ϰ� �ٸ� Ŭ���̾�Ʈ���� �˸�

// Ŭ���̾�Ʈ ���� �Լ�
void removeClient(SOCKET clientSocket) {
    EnterCriticalSection(&csClient);  // Ŭ���̾�Ʈ ��� ����ȭ
    auto clientIt = std::find_if(clientInfos.begin(), clientInfos.end(), [&](const ClientInfo& client) {
        return client.clientSocket == clientSocket;
        });

    if (clientIt != clientInfos.end()) {
        int clientId = clientIt->id;
        clientInfos.erase(clientIt);

        // �÷��̾� ��Ͽ��� �ش� Ŭ���̾�Ʈ�� �÷��̾� ����
        EnterCriticalSection(&csPlayer);  // �÷��̾� ��� ����ȭ
        auto playerIt = std::find_if(players.begin(), players.end(), [&](const Player& player) {
            return player.id == clientId;
            });

        if (playerIt != players.end()) {
            players.erase(playerIt);
        }
        LeaveCriticalSection(&csPlayer);  // �÷��̾� ��� ����ȭ ����
    }
    LeaveCriticalSection(&csClient);  // Ŭ���̾�Ʈ ��� ����ȭ ����
}
// Ŭ���̾�Ʈ ��û ó�� ������
void execute() {
    while (true) {
        EnterCriticalSection(&csQueue);
        if (!requestQueue.empty()) {
            QueueBasket req = requestQueue.front();
            requestQueue.pop_front();
            LeaveCriticalSection(&csQueue);

            if (req.requestType == LOGIN_TRY) {
                ClientInfo newClient;
                EnterCriticalSection(&csClientCount);
                newClient.id = client_count++;
                LeaveCriticalSection(&csClientCount);
                newClient.clientSocket = req.clientSocket;

                // �÷��̾� ���� �� �ʱ�ȭ
                Player newPlayer;
                newPlayer.id = newClient.id;
                newPlayer.shape.setPosition(rand() % 800, rand() % 600); // ���� ��ġ
                int randomIndex = std::rand() % colors.size();
                
                newPlayer.color = randomIndex; // ���� ����
                

                EnterCriticalSection(&csQueue);
                clientInfos.push_back(newClient);
                players.push_back(newPlayer);
                LeaveCriticalSection(&csQueue);
                printf("�Ϸ�\n");

                // Ŭ���̾�Ʈ���� LOGIN_SUCCESS ���� ����
                uint successMsg = LOGIN_SUCCESS;
                successMsg = hton_requestType(successMsg);
                //send(req.clientSocket, reinterpret_cast<char*>(&successMsg), static_cast<int>(sizeof(successMsg)), 0);
                printf("Client [%s] �α��� ���� (LOGIN_SUCCESS ����)\n", req.clientName);

                // ��� Ŭ���̾�Ʈ���� �÷��̾� ���� ����
                sendPlayerInfoToAllClients();
                for (Player p : players) {
                    printf("id: %d\n", p.id);
                }
            }
        }
        else {
            LeaveCriticalSection(&csQueue);
            //Sleep(10);
        }
    }
}

// Ŭ���̾�Ʈ ������ �Լ�
DWORD WINAPI ProcessClient(LPVOID arg) {
    SOCKET client_sock = (SOCKET)arg;
    char buf[BUFSIZE];
    int retval;
    while (true) {
        // ù ��°�� LOGIN_TRY ��û�� ����
        retval = recv(client_sock, buf, sizeof(uint), 0); // ó������ ��û Ÿ�Ը� ����
        if (retval <= 0) {
            removeClient(client_sock); // Ŭ���̾�Ʈ�� ���� �� ó��
            closesocket(client_sock);
            return 0;
        }

        // RequestType�� ����
        uint requestType;
        memcpy(&requestType, buf, sizeof(uint));
        requestType = ntoh_requestType(requestType);
        if (requestType == LOGIN_TRY) {
            // �� ��°�� Ŭ���̾�Ʈ �̸��� ����
            retval = recv(client_sock, buf, sizeof(buf), 0); // Ŭ���̾�Ʈ �̸��� ����
            if (retval <= 0) {
                removeClient(client_sock); // Ŭ���̾�Ʈ�� ���� �� ó��
                closesocket(client_sock);
                printf("���޾��� �̸�\n");
                return 0;
            }

            QueueBasket req;
            req.requestType = LOGIN_TRY;
            req.clientSocket = client_sock;
            strncpy(req.clientName, buf, sizeof(req.clientName) - 1); // Ŭ���̾�Ʈ �̸� ����
            req.clientName[sizeof(req.clientName) - 1] = '\0'; // ���ڿ� ���� �� ���� �߰�

            // ť�� ��û�� �߰�
            EnterCriticalSection(&csQueue);
            requestQueue.push_back(req);
            LeaveCriticalSection(&csQueue);
            printf("���ú� �ް� �Ѱ��� %d\n", requestType);
        }
    }
    closesocket(client_sock);
    return 0;
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

    InitializeCriticalSection(&csQueue);
    InitializeCriticalSection(&csClient);
    InitializeCriticalSection(&csPlayer);
    InitializeCriticalSection(&csClientCount);

    std::thread executeThread(execute);
    executeThread.detach();

    while (1) {
        SOCKET client_sock = accept(listen_sock, nullptr, nullptr);
        CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
    }

    DeleteCriticalSection(&csQueue);
    closesocket(listen_sock);
    WSACleanup();
    return 0;
}
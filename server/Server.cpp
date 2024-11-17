#include "server.h"
#include <thread>
#include <string>
#include <cstring>
#include <algorithm>  // 정렬을 위해 추가
#include <SFML/System.hpp> // SFML 벡터 사용을 위한 헤더
#define SERVERPORT 9000
#define BUFSIZE 512

std::vector<ClientInfo> clientInfos;
std::deque<QueueBasket> requestQueue;
std::vector<Player> players;
CRITICAL_SECTION csClientCount;
CRITICAL_SECTION csQueue;   // 큐 관련 임계 영역
CRITICAL_SECTION csClient;  // 클라이언트 목록 관련 임계 영역
CRITICAL_SECTION csPlayer;  // 플레이어 목록 관련 임계 영역
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
    EnterCriticalSection(&csQueue);
    for (const auto& client : clientInfos) {
        send(client.clientSocket, data, dataSize, 0);
    }
    LeaveCriticalSection(&csQueue);
}

// 플레이어 정보를 모든 클라이언트에게 전송
void sendPlayerInfoToAllClients() {
    // 패킷 타입과 크기를 전송
    uint packetType = hton_requestType(LOGIN_SUCCESS);
    sendToAllClients(reinterpret_cast<char*>(&packetType), sizeof(packetType));

    // 플레이어 수를 전송
    uint playerCount = htonl(players.size());
    sendToAllClients(reinterpret_cast<char*>(&playerCount), sizeof(playerCount));

    // 플레이어 정보들을 하나의 큰 버퍼에 담아서 전송
    std::vector<char> allPlayerData;
    for (const auto& player : players) {
        PlayerInfo playerInfo;
        playerInfo.id = htonl(player.id);
        playerInfo.color = htonl(player.color);
        playerInfo.posX = player.Position().x;
        playerInfo.posY = player.Position().y;

        // 각 PlayerInfo를 큰 버퍼에 추가
        allPlayerData.insert(allPlayerData.end(), reinterpret_cast<char*>(&playerInfo), reinterpret_cast<char*>(&playerInfo) + sizeof(PlayerInfo));
    }

    // 모든 플레이어 데이터를 한 번에 전송
    sendToAllClients(allPlayerData.data(), allPlayerData.size());
}

// 클라이언트를 목록에서 삭제하고 다른 클라이언트에게 알림

// 클라이언트 삭제 함수
void removeClient(SOCKET clientSocket) {
    EnterCriticalSection(&csClient);  // 클라이언트 목록 동기화
    auto clientIt = std::find_if(clientInfos.begin(), clientInfos.end(), [&](const ClientInfo& client) {
        return client.clientSocket == clientSocket;
        });

    if (clientIt != clientInfos.end()) {
        int clientId = clientIt->id;
        clientInfos.erase(clientIt);

        // 플레이어 목록에서 해당 클라이언트의 플레이어 삭제
        EnterCriticalSection(&csPlayer);  // 플레이어 목록 동기화
        auto playerIt = std::find_if(players.begin(), players.end(), [&](const Player& player) {
            return player.id == clientId;
            });

        if (playerIt != players.end()) {
            players.erase(playerIt);
        }
        LeaveCriticalSection(&csPlayer);  // 플레이어 목록 동기화 종료
    }
    LeaveCriticalSection(&csClient);  // 클라이언트 목록 동기화 종료
}
// 클라이언트 요청 처리 쓰레드
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

                // 플레이어 생성 및 초기화
                Player newPlayer;
                newPlayer.id = newClient.id;
                newPlayer.shape.setPosition(rand() % 800, rand() % 600); // 랜덤 위치
                int randomIndex = std::rand() % colors.size();
                
                newPlayer.color = randomIndex; // 랜덤 색상
                

                EnterCriticalSection(&csQueue);
                clientInfos.push_back(newClient);
                players.push_back(newPlayer);
                LeaveCriticalSection(&csQueue);
                printf("완료\n");

                // 클라이언트에게 LOGIN_SUCCESS 응답 전송
                uint successMsg = LOGIN_SUCCESS;
                successMsg = hton_requestType(successMsg);
                //send(req.clientSocket, reinterpret_cast<char*>(&successMsg), static_cast<int>(sizeof(successMsg)), 0);
                printf("Client [%s] 로그인 성공 (LOGIN_SUCCESS 전송)\n", req.clientName);

                // 모든 클라이언트에게 플레이어 정보 전송
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

// 클라이언트 쓰레드 함수
DWORD WINAPI ProcessClient(LPVOID arg) {
    SOCKET client_sock = (SOCKET)arg;
    char buf[BUFSIZE];
    int retval;
    while (true) {
        // 첫 번째로 LOGIN_TRY 요청을 받음
        retval = recv(client_sock, buf, sizeof(uint), 0); // 처음에는 요청 타입만 받음
        if (retval <= 0) {
            removeClient(client_sock); // 클라이언트가 나갈 때 처리
            closesocket(client_sock);
            return 0;
        }

        // RequestType을 받음
        uint requestType;
        memcpy(&requestType, buf, sizeof(uint));
        requestType = ntoh_requestType(requestType);
        if (requestType == LOGIN_TRY) {
            // 두 번째로 클라이언트 이름을 받음
            retval = recv(client_sock, buf, sizeof(buf), 0); // 클라이언트 이름을 받음
            if (retval <= 0) {
                removeClient(client_sock); // 클라이언트가 나갈 때 처리
                closesocket(client_sock);
                printf("못받았음 이름\n");
                return 0;
            }

            QueueBasket req;
            req.requestType = LOGIN_TRY;
            req.clientSocket = client_sock;
            strncpy(req.clientName, buf, sizeof(req.clientName) - 1); // 클라이언트 이름 복사
            req.clientName[sizeof(req.clientName) - 1] = '\0'; // 문자열 끝에 널 종료 추가

            // 큐에 요청을 추가
            EnterCriticalSection(&csQueue);
            requestQueue.push_back(req);
            LeaveCriticalSection(&csQueue);
            printf("리시브 받고 넘겨줌 %d\n", requestType);
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
#include "..\\..\\Common.h"
#include <thread>
#include <string>
#include <cstring>
#include <algorithm>  // ������ ���� �߰�

#define SERVERPORT 9000
#define BUFSIZE 512

std::vector<ClientInfo> clientInfos;
std::deque<QueueBasket> requestQueue;
CRITICAL_SECTION csQueue;

int client_count = 0;

RequestType hton_requestType(RequestType type) {
    return static_cast<RequestType>(htonl(static_cast<int>(type)));
}

RequestType ntoh_requestType(RequestType type) {
    return static_cast<RequestType>(ntohl(static_cast<int>(type)));
}

// ClientInfo ��ȯ �Լ� (��Ʈ��ũ ����Ʈ ����)
void hton_clientInfo(ClientInfo& client) {
    client.id = htonl(client.id);
}

void ntoh_clientInfo(ClientInfo& client) {
    client.id = ntohl(client.id);
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
                newClient.id = client_count++;
                //strncpy(newClient.name, req.clientName, sizeof(newClient.name));
                newClient.clientSocket = req.clientSocket;
                hton_clientInfo(newClient);
                EnterCriticalSection(&csQueue);
                clientInfos.push_back(newClient);
                LeaveCriticalSection(&csQueue);

                // Ŭ���̾�Ʈ���� LOGIN_SUCCESS ���� ����
                RequestType successMsg = LOGIN_SUCCESS;
                successMsg = hton_requestType(successMsg);
                send(req.clientSocket, (char*)&successMsg, sizeof(successMsg), 0);
                printf("Client [%s] �α��� ���� (LOGIN_SUCCESS ����)\n", req.clientName);


            }
        }
        else {
            LeaveCriticalSection(&csQueue);
            //Sleep(10);
        }
    }
}

// Ŭ���̾�Ʈ ������ �Լ�
// Ŭ���̾�Ʈ ������ �Լ�
DWORD WINAPI ProcessClient(LPVOID arg) {
    SOCKET client_sock = (SOCKET)arg;
    char buf[BUFSIZE];
    int retval;

    // ù ��°�� LOGIN_TRY ��û�� ����
    retval = recv(client_sock, buf, sizeof(RequestType), 0); // ó������ ��û Ÿ�Ը� ����
    if (retval <= 0) {
        closesocket(client_sock);
        return 0;
    }

    // RequestType�� ����
    RequestType requestType;
    memcpy(&requestType, buf, sizeof(RequestType));
    requestType = ntoh_requestType(requestType);

    if (requestType == LOGIN_TRY) {
        // �� ��°�� Ŭ���̾�Ʈ �̸��� ����
        retval = recv(client_sock, buf, sizeof(buf), 0); // Ŭ���̾�Ʈ �̸��� ����
        if (retval <= 0) {
            closesocket(client_sock);
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

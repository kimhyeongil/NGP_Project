#include "server.h"

#define SERVERPORT 9000
#define BUFSIZE    512

int client_count = 0;        // Ŭ���̾�Ʈ �� ����
CRITICAL_SECTION cs;

// Ŭ���̾�Ʈ�� ���� ������ ó���ϴ� ������ �Լ�
DWORD WINAPI ProcessClient(LPVOID arg) {
    SOCKET client_sock = (SOCKET)arg;
    struct sockaddr_in clientaddr;
    char addr[INET_ADDRSTRLEN];
    int addrlen = sizeof(clientaddr);
    int retval;
    char buf[BUFSIZE];
    FILE* fp = NULL;
    long file_size = 0;
    long total_received = 0;
    int len;
    int id;
    Player play;
    // Ŭ���̾�Ʈ ID ���� (�Ӱ迵�� ��ȣ)
    EnterCriticalSection(&cs);
    id = client_count++;
    LeaveCriticalSection(&cs);

    // Ŭ���̾�Ʈ ���� ���
    getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

    // ���� �̸� �ޱ�
    if (recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL) <= 0) {
        printf("���� �̸� ���� ���� ����\n");
        closesocket(client_sock);
        return 1;
    }

    retval = recv(client_sock, buf, len, MSG_WAITALL);
    if (retval <= 0) {
        printf("���� �̸� ���� ����\n");
        closesocket(client_sock);
        return 1;
    }
    buf[retval] = '\0';  // ���� �̸� ���� ���� �߰�

    // ���� ����
    fp = fopen(buf, "wb");
    if (fp == NULL) {
        printf("���� ���� ����: %s\n", buf);
        closesocket(client_sock);
        return 1;
    }

    // ���� ũ�� �ޱ�
    if (recv(client_sock, (char*)&file_size, sizeof(file_size), MSG_WAITALL) <= 0) {
        printf("���� ũ�� ���� ����\n");
        fclose(fp);
        closesocket(client_sock);
        return 1;
    }

    // ���� ������ �ޱ� �� ����
    total_received = 0;
    while (total_received < file_size) {
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval <= 0) {
            printf("���� ������ ���� ����\n");
            break;
        }

        fwrite(buf, 1, retval, fp);
        total_received += retval;

        // ���ŷ� ���
        double percentage = (double)total_received / file_size * 100;

        
    }

    if (total_received == file_size) {
        printf(" ->���� ���� �Ϸ�\n", addr);
    }
    else {
        printf(" ->���� ���� �� ���� �߻�\n", addr);
    }

    // ���� �ݱ�
    fclose(fp);
    // Ŭ���̾�Ʈ ���� �ݱ�
    closesocket(client_sock);

    return 0;
}

int main(int argc, char* argv[]) {
    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup ����\n");
        return 1;
    }

    // ���� ����
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        printf("���� ���� ����\n");
        WSACleanup();
        return 1;
    }

    // ���� �ּ� ���� �� ���ε�
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) {
        printf("bind ����\n");
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }

    // ���� ��� ����
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) {
        printf("listen ����\n");
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }

    // Ŭ���̾�Ʈ ó��
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);
    InitializeCriticalSection(&cs);
    HANDLE hThread;
   

    while (1) {
        // Ŭ���̾�Ʈ ���� ����
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            printf("Ŭ���̾�Ʈ ���� ����\n");
            continue;
        }

        // ������ �����Ͽ� Ŭ���̾�Ʈ ó��
        hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) {
            printf("������ ���� ����\n");
            closesocket(client_sock);
        }
        else {
            CloseHandle(hThread);  // ������ �ڵ� �ݱ�
        }
    }

    // ���� ���� �ݱ�
    closesocket(listen_sock);

    // �Ӱ迵�� ����
    DeleteCriticalSection(&cs);

    // ���� ����
    WSACleanup();
    return 0;
}





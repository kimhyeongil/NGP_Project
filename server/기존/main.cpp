#include "server.h"
#define SERVERPORT 9000
#define BUFSIZE 512

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
#include "server.h"

#define SERVERPORT 9000
#define BUFSIZE    512

int client_count = 0;        // 클라이언트 수 관리
CRITICAL_SECTION cs;

// 클라이언트와 파일 전송을 처리하는 스레드 함수
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
    // 클라이언트 ID 결정 (임계영역 보호)
    EnterCriticalSection(&cs);
    id = client_count++;
    LeaveCriticalSection(&cs);

    // 클라이언트 정보 얻기
    getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
    inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

    // 파일 이름 받기
    if (recv(client_sock, (char*)&len, sizeof(int), MSG_WAITALL) <= 0) {
        printf("파일 이름 길이 수신 실패\n");
        closesocket(client_sock);
        return 1;
    }

    retval = recv(client_sock, buf, len, MSG_WAITALL);
    if (retval <= 0) {
        printf("파일 이름 수신 실패\n");
        closesocket(client_sock);
        return 1;
    }
    buf[retval] = '\0';  // 파일 이름 종료 문자 추가

    // 파일 열기
    fp = fopen(buf, "wb");
    if (fp == NULL) {
        printf("파일 열기 실패: %s\n", buf);
        closesocket(client_sock);
        return 1;
    }

    // 파일 크기 받기
    if (recv(client_sock, (char*)&file_size, sizeof(file_size), MSG_WAITALL) <= 0) {
        printf("파일 크기 수신 실패\n");
        fclose(fp);
        closesocket(client_sock);
        return 1;
    }

    // 파일 데이터 받기 및 저장
    total_received = 0;
    while (total_received < file_size) {
        retval = recv(client_sock, buf, BUFSIZE, 0);
        if (retval <= 0) {
            printf("파일 데이터 수신 실패\n");
            break;
        }

        fwrite(buf, 1, retval, fp);
        total_received += retval;

        // 수신률 출력
        double percentage = (double)total_received / file_size * 100;

        
    }

    if (total_received == file_size) {
        printf(" ->파일 전송 완료\n", addr);
    }
    else {
        printf(" ->파일 전송 중 오류 발생\n", addr);
    }

    // 파일 닫기
    fclose(fp);
    // 클라이언트 소켓 닫기
    closesocket(client_sock);

    return 0;
}

int main(int argc, char* argv[]) {
    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    // 소켓 생성
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        printf("소켓 생성 실패\n");
        WSACleanup();
        return 1;
    }

    // 서버 주소 설정 및 바인드
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) {
        printf("bind 실패\n");
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }

    // 소켓 듣기 시작
    retval = listen(listen_sock, SOMAXCONN);
    if (retval == SOCKET_ERROR) {
        printf("listen 실패\n");
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }

    // 클라이언트 처리
    SOCKET client_sock;
    struct sockaddr_in clientaddr;
    int addrlen = sizeof(clientaddr);
    InitializeCriticalSection(&cs);
    HANDLE hThread;
   

    while (1) {
        // 클라이언트 연결 수락
        client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
        if (client_sock == INVALID_SOCKET) {
            printf("클라이언트 연결 실패\n");
            continue;
        }

        // 스레드 생성하여 클라이언트 처리
        hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)client_sock, 0, NULL);
        if (hThread == NULL) {
            printf("스레드 생성 실패\n");
            closesocket(client_sock);
        }
        else {
            CloseHandle(hThread);  // 스레드 핸들 닫기
        }
    }

    // 서버 소켓 닫기
    closesocket(listen_sock);

    // 임계영역 삭제
    DeleteCriticalSection(&cs);

    // 윈속 종료
    WSACleanup();
    return 0;
}





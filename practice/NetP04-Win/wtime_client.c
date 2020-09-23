/*
파일명 : time_client.c
기  능 : time 서비스를 요구하는 TCP(연결형) 클라이언트
컴파일 : cc - o time_client time_client.c
사용법 : time_client[ip][port] // default는 127.0.0.1 30000
*/
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

WSADATA wsadata;
int	main_socket;

void exit_callback(int sig)
{
    closesocket(main_socket);
    WSACleanup();
    exit(0);
}

void init_winsock()
{
    WORD sversion;
    u_long iMode = 1;

    // winsock 사용을 위해 필수적임
    signal(SIGINT, exit_callback);
    sversion = MAKEWORD(1, 1);
    WSAStartup(sversion, &wsadata);
}

#define BUF_LEN 128
#define TIME_SERVER "127.0.0.1"
#define TIME_PORT "30000"

void main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in server;
    char* haddr;
    char buf[BUF_LEN + 1] = { 0 };
    char* ip_addr = TIME_SERVER, * port_no = TIME_PORT;

    init_winsock();
     
    if (argc == 3) {
        ip_addr = argv[1];
        port_no = argv[2];
    }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("can't create socket\n");
        exit(0);
    }
    main_socket = sock;
    /* time 서버의 소켓주소 구조체 작성 */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_port = htons(atoi(port_no));

    printf("Connecting %s %s\n", ip_addr, port_no);
    /* 연결요청 */
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("can't connect.\n");
        exit(1);
    }

    if (recv(sock, buf, BUF_LEN, 0) == -1) {
        printf("recv error\n");
        exit(1);
    }
    printf("Time information from server is %s", buf);
    closesocket(sock);
    return(0);
}


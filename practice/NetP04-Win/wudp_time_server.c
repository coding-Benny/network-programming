/*
파일명 : udp_time_server.c
기  능 : time 서비스를 수행하는 UDP 서버
컴파일 : cc -w -o time_server time_server.c
사용법 : udp_time_server [port]
*/

#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

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

#define TIME_SERVER "0.0.0.0"
#define TIME_PORT "30000"

void main(int argc, char* argv[]) {
	struct sockaddr_in server, client;
	int sock;                       /* 소켓번호 */
	int buf_len, client_len;
	char buf[256];
	time_t today;
	char* ip_addr = TIME_SERVER, * port_no = TIME_PORT;

	if (argc == 2) // default 30000 변경할 수 있도록
		port_no = argv[1];

    init_winsock();

	/* 소켓 생성 */
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(1);
	}
    main_socket = sock;
    /* server 세팅 */
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr(ip_addr);
    server.sin_port = htons(atoi(port_no));
    /* bind() 호출 */
    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Server: Can't bind local address.\n");
        exit(1);
    }

    memset(&client, 0, sizeof(client));
    printf("Waiting\n");
    while (1) {
        /* 연결요청을 기다림 */
        client_len = sizeof(client);
        buf_len = recvfrom(sock, buf, 256, 0, (struct sockaddr*)&client, &client_len);
        if (buf_len < 0) {
            printf("Server: recvfrom failed.\n");
            //perror(errno);
            exit(1);
        }

        printf("Server : A client data recvfrom msg = %s.\n", buf);
        printf("From %s : %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        time(&today);
        strcpy(buf, ctime(&today));
        printf("Server time=%s", buf);
        sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr*)&client, sizeof(client));
    }
}


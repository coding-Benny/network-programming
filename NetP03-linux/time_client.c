/*
 파일명 : time_client.c
 기  능 : time 서비스를 요구하는 TCP(연결형) 클라이언트
 컴파일 : cc -o time_client time_client.c
 사용법 : time_client [ip] [port] // default는 127.0.0.1 30000
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <unistd.h>
#define BUF_LEN 128
#define TIME_SERVER "127.0.0.1"
#define TIME_PORT "30000"

void main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in server;
	char *haddr;
	char buf[BUF_LEN+1] = {0};
	char *ip_addr = TIME_SERVER, *port_no = TIME_PORT;

	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}
	if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("can't create socket\n");
		exit(0);
	}
	/* time 서버의 소켓주소 구조체 작성 */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_port = htons(atoi(port_no));

	/* 연결요청 */
	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("can't connect.\n");
		exit(1);
	}
	
	if (recv(sock, buf, BUF_LEN, 0) == -1) {
		printf("recv error\n");
		exit(1);
	}
	printf("Time information from server is %s", buf);
	close(sock);
	return(0);
}

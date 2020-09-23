/*
파일명 : time_server.c 
기  능 : time 서비스를 수행하는 서버
컴파일 : cc -o time_server time_server.c
사용법 : time_server [30000]
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>

#define TIME_PORT	"30000"

void main(int argc, char *argv[]) {
	struct sockaddr_in server, client;
	int sock, sock2;			/* 소켓번호 */
	int len;
	char buf[256];
	unsigned int set = 1;
	time_t today;
	char *port_no = TIME_PORT;

	if (argc == 2)
		port_no = argv[1];
	/* 소켓 생성 */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(1);
	}

	//printf("sock = %d\n", sock);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&set, sizeof(set));

	/* server 세팅 */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(atoi(port_no));

	/* bind() 호출 */
	if(bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(1);
	}
	/* 소켓을 수동 대기모드로 세팅 */
	listen(sock, 5);

	/* iterative  time 서비스 수행 */
	printf("Server : waiting connection request.\n");
	len = sizeof(client);

	while (1) {
		/* 연결요청을 기다림 */
		sock2 = accept(sock, (struct sockaddr *)&client, &len);
		//printf("sock2 = %d\n", sock2);
		if(sock2 < 0) {
			printf("Server: accept failed.\n");
			exit(1);
		}

		printf("Server : A client connected.\n");
		printf("From %s : %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		time(&today);
		strcpy(buf, ctime(&today));
		printf("Server time=%s", buf);
		send(sock2, buf, strlen(buf) + 1, 0);
		close(sock2);
	}
}


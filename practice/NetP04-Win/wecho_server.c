/*
파일명 : wecho_server.c
기  능 : echo 서비스를 수행하는 서버
컴파일 : cc -o wecho_server wecho_server.c
사용법 : wecho_server [port]
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
#define ECHO_SERVER "0.0.0.0"
#define ECHO_PORT "30000"

int main(int argc, char* argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;			/* 소켓번호 */
	int len, msg_size;
	char buf[BUF_LEN + 1];
	unsigned int set = 1;
	char* ip_addr = ECHO_SERVER, * port_no = ECHO_PORT;

	if (argc == 2) {
		port_no = argv[1];
	}

	init_winsock();

	/* 소켓 생성 */
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
	main_socket = server_fd;

	printf("echo_server waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));

	/* server_addr을 '\0'으로 초기화 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	/* server_addr 세팅 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	/* bind() 호출 */
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}

	/* 소켓을 수동 대기모드로 세팅 */
	listen(server_fd, 5);

	/* iterative  echo 서비스 수행 */
	printf("Server : waiting connection request.\n");
	len = sizeof(client_addr);

	while (1) {
		/* 연결요청을 기다림 */
		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
		if (client_fd < 0) {
			printf("Server: accept failed.\n");
			exit(0);
		}

		printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		printf("client_fd = %d\n", client_fd);

		//while (1) {
		msg_size = recv(client_fd, buf, BUF_LEN, 0);
		if (msg_size <= 0) {
			printf("recv error\n");
			break;
		}
		buf[msg_size] = '\0'; // 문자열 끝에 NULL를 추가하기 위함
		printf("Received len=%d : %s", msg_size, buf);
		msg_size = send(client_fd, buf, msg_size, 0);
		if (msg_size <= 0) {
			printf("send error\n");
			break;
		}
		printf("Sending len=%d : %s", msg_size, buf);
		//}
		closesocket(client_fd); // close(client_fd);
	}
	closesocket(server_fd); // close(client_fd);
	return(0);
}


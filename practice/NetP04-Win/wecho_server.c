/*
파일명 : wecho_server.c
기  능 : echo 서비스를 수행하는 서버
컴파일 : cc -o wecho_server wecho_server.c
사용법 : wecho_server [port]
*/
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

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
#define USER_ID "hansung"
#define USER_PWD "computer"

int main(int argc, char* argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;			/* 소켓번호 */
	int len, msg_size = -1;
	char buf[BUF_LEN + 1];
	unsigned int set = 1;
	char* ip_addr = ECHO_SERVER, *port_no = ECHO_PORT;
	char* user_id = USER_ID, *user_pwd = USER_PWD, *input_id;
	char* welcome_msg = "Welcome to Server!!";

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

	printf("echo_server5 waiting connection..\n");
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

		msg_size = send(client_fd, welcome_msg, BUF_LEN, 0);
		if (msg_size <= 0) {
			printf("send error\n");
			break;
		}
		printf("Sending : %s\n", welcome_msg);

		/* 로그인 */
		while (1) {
			int i=0, j=0, k=0;
			char *s = buf;
			char *info[2], *val[2];
			char *id_info, *pwd_info, *input_pwd, *p;
			char str[BUF_LEN + 1] = { 0 };
			char res[BUF_LEN + 1] = { 0 };
			char welcome_user[BUF_LEN + 1] = { 0 };

			msg_size = recv(client_fd, buf, BUF_LEN, 0);

			if (msg_size <= 0) {
				printf("recv error\n");
				break;
			}
			buf[msg_size] = '\0'; // 문자열 끝에 NULL를 추가하기 위함

			printf("Received %s\n", buf);

			/* 문자열 처리 통해 입력한 id와 password 추출 */
			strcpy(str, s);
			p = strtok(str, " ");
			while (p) {
				info[i++] = p;
				p = strtok(NULL, " ");
			}
			id_info = info[0]; pwd_info = info[1];
			p = strtok(id_info, "=");
			while (p) {
				info[j++] = p;
				p = strtok(NULL, "=");
			}
			input_id = info[1];
			p = strtok(pwd_info, "=");
			while (p) {
				info[k++] = p;
				p = strtok(NULL, "=");
			}
			input_pwd = info[1];

			/* 문자열 비교 결과에 따른 응답 설정 */
			if (strcmp(input_id, user_id) == 0 && strcmp(input_pwd, user_pwd) == 0) {	// valid
				sprintf(res, "200 Welcome %s!!\n", input_id);
			}
			else {
				if (strcmp(input_id, user_id) != 0) {	// invalid id(+password)
					sprintf(res, "401 Invalid ID\n");
				}
				else {	// valid id, invalid pwd
					sprintf(res, "402 Invalid Password\n");
				}
			}

			if (strstr(res, "200") != NULL) {
				strncpy(welcome_user, res + 4, strlen(res));
				msg_size = send(client_fd, welcome_user, msg_size, 0);
			}
			else
				msg_size = send(client_fd, res, msg_size, 0);

			if (msg_size <= 0) {
				printf("send error\n");
				break;
			}
			printf("Sending len=%d : %s", msg_size, res);

			if (strstr(res, "Welcome") != NULL)
				break;
		}

		while (1) {
			int opt = -1;
			char *s = buf;
			char req[BUF_LEN + 1];
			char menu[BUF_LEN + 1] = "";
			char msg[BUF_LEN + 1] = "";

			msg_size = recv(client_fd, buf, BUF_LEN, 0);

			if (msg_size <= 0) {
				printf("recv error\n");
				break;
			}
			buf[msg_size] = '\0'; // 문자열 끝에 NULL를 추가하기 위함

			strcpy(req, s);
			strncpy(menu, req, 1);
			opt = atoi(menu);
			strncpy(msg, req + 2, strlen(req));
			printf("Received len=%d : %s", msg_size, buf);
			if (strstr(buf, "4") != NULL) {
				printf("\nSession finished for %s.\n", input_id);
			}

			switch (opt)
			{
			case 1:
				// 모든 문자열을 대문자로 변환
				for (int i = 0; i < strlen(msg); i++) {
					msg[i] = toupper(msg[i]);
				}
				break;
			case 2:
				// 모든 문자열을 대문자로 변환
				for (int i = 0; i < strlen(msg); i++) {
					msg[i] = tolower(msg[i]);
				}
				break;
			case 3:
				// 대/소문자 상호 교환
				for (int i = 0; i < strlen(msg); i++) {
					if (islower(msg[i]))
						msg[i] = toupper(msg[i]);
					else
						msg[i] = tolower(msg[i]);
				}
				break;
			case 4:
				printf("\n");
				break;
			default:
				break;
			}

			if (opt != 4) {
				msg_size = send(client_fd, msg, msg_size, 0);
				if (msg_size <= 0) {
					printf("send error\n");
					break;
				}
				printf("Sending len=%d : %s", msg_size, msg);
			}
			else {
				break;
			}
		}
		closesocket(client_fd); // close(client_fd);
	}
	closesocket(server_fd); // close(client_fd);
	return(0);
}


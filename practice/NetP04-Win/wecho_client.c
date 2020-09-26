/*
 파일명 : echo_client.c
 기  능 : echo 서비스를 요구하는 TCP(연결형) 클라이언트
 컴파일 : cc -o echo_client echo_client.c
 사용법 : echo_client [host] [port]
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

#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"
#define BUF_LEN 128

int main(int argc, char* argv[]) {
	int c, s, n, len_in, len_out;
	struct sockaddr_in server_addr;
	char* ip_addr = ECHO_SERVER, *port_no = ECHO_PORT;
	char user_info[BUF_LEN + 1] = { 0 };
	char id[BUF_LEN + 1] = { 0 };
	char pwd[BUF_LEN + 1] = { 0 };
	char buf[BUF_LEN + 1] = { 0 };
	char req[BUF_LEN + 1] = { 0 };

	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}

	init_winsock();

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("can't create socket\n");
		exit(0);
	}
	main_socket = s;


	/* echo 서버의 소켓주소 구조체 작성 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* 연결요청 */
	printf("Connecting %s %s\n", ip_addr, port_no);	// Connecting 127.0.0.1 30000

	if (connect(s, (struct sockaddr*)&server_addr,
		sizeof(server_addr)) < 0) {
		printf("can't connect.\n");
		exit(0);
	}

	if ((n = recv(s, buf, BUF_LEN, 0)) < 0) {
		printf("recv error\n");
		exit(0);
	}
	buf[n] = '\0'; // 문자열 끝에 NULL 추가
	printf("Received : %s\n", buf);

	while (1) {
		/* 로그인 */
		printf("ID : ");
		scanf("%s", id);
		printf("Password : ");
		scanf("%s", pwd);
		
		sprintf(user_info, "id=%s pass=%s", id, pwd);

		/* echo 서버로 메시지 송신 */
		if (send(s, user_info, BUF_LEN, 0) < 0) {
			printf("send error\n");
			exit(0);
		}

		if ((n = recv(s, buf, BUF_LEN, 0)) < 0) {
			printf("recv error\n");
			exit(0);
		}
		buf[n] = '\0'; // 문자열 끝에 NULL 추가
		printf("%s", buf);
		if (strstr(buf, "Welcome") != NULL)
			break;
	}

	while (1) {
		printf("*** 대/소문자 변환 메뉴입니다. ***\n");
		printf(" (1) 모두 대문자 변환\n");
		printf(" (2) 모두 소문자 변환\n");
		printf(" (3) 대>소 소>대 변환\n");
		printf(" (4) 종료\n");
		/* 메뉴 선택 */
		printf("선택하세요 : ");
		scanf("%s", req);
		while ((c = getchar()) != EOF && c != '\n');

		if (strcmp(req, "4") == 0) {
			if (send(s, req, BUF_LEN, 0) < 0) {
				printf("send error\n");
			}
			exit(0);
		}

		/* 문자열 입력 */
		printf("Input string : ");
		if (fgets(buf, BUF_LEN, stdin)) { // gets(buf);
			len_out = strlen(buf);
			buf[BUF_LEN] = '\0';
		}
		else {
			printf("fgets error\n");
			exit(0);
		}

		sprintf(req, "%s %s", req, buf);

		/* echo 서버로 메시지 송신 */
		if (send(s, req, BUF_LEN, 0) < 0) {
			printf("send error\n");
			exit(0);
		}

		if ((n = recv(s, buf, BUF_LEN, 0)) < 0) {
			printf("recv error\n");
			exit(0);
		}

		buf[n] = '\0'; // 문자열 끝에 NULL 추가
		printf("Received len=%d : %s\n", n, buf);
	}
	closesocket(s);
	return(0);
}
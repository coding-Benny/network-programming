/*
Client.c
2020년 네트워크프로그래밍 중간고사 실습문제 제출용
chat_client1.c 기반
*/
#ifdef WIN32
#define  _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)
#pragma warning(disable:4267)
#pragma warning(disable:4244)
#pragma warning(disable:6031)

#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#endif

#ifdef WIN32
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
#endif

#define CHAT_SERVER "127.0.0.1"
#define CHAT_PORT "30000"
#define BUF_LEN 128

#define EXIT	"004"
void do_recv(char* buf);
void do_keyboard(int s);
int mode; // 1: 주문 client, 2: 주방

int main(int argc, char* argv[]) {
	char buf[BUF_LEN + 1];
	int s, n;
	struct sockaddr_in server_addr;
	char* ip_addr = CHAT_SERVER, * port_no = CHAT_PORT;
	struct timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = 1000;

	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}

#ifdef WIN32
	printf("Windows : ");
	init_winsock();
#else // Linux
	printf("Linux : ");
#endif 

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("can't create socket\n");
		exit(0);
	}
#ifdef WIN32
	main_socket = s;
#endif 
	// 수정하지 말 것.
	do {
		printf("Client 모드 선택 (1: 주문, 2: 주방) : ");
		scanf("%d", &mode); getchar();
	} while (mode != 1 && mode != 2);

	/* 채팅 서버의 소켓주소 구조체 server_addr 초기화 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* 연결요청 */
	printf("Client connecting %s %s\n", ip_addr, port_no);

	/* 연결요청 */
	if (connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		exit(0);
	}
	else {
		printf("서버에 접속되었습니다. \n");
	}
#ifdef WIN32
	u_long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode); // 소켓을 non-blocking 으로 만든다.
	while (1) {
		memset(buf, 0, BUF_LEN);
		// Non-blocking read이므로 데이터가 앖으면 기다리지 않고 0으로 return
		n = recv(s, buf, BUF_LEN, 0);
		if (n > 0) { // non-blocking read
			do_recv(buf);
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK) {
			printf("recv error\n"); // server 가 종료되었거나 네트워크 오류
			break;
		}
		if (kbhit()) { // key 가 눌려있으면 read key --> write to chat server
			do_keyboard(s);
		}
		Sleep(10); // Non-blocking I/O CPU 부담을 줄인다.
	}
#else
	int maxfdp;
	fd_set read_fds;
	maxfdp = s + 1; // socket은 항상 0 보다 크게 할당된다.
	FD_ZERO(&read_fds);
	while (1) {
		memset(buf, 0, BUF_LEN);
		FD_SET(0, &read_fds); // stdin:0 표준입력은 file 번호 = 0 이다.
		FD_SET(s, &read_fds); // server 와 견결된 socket 번호

		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
			printf("select error\n");
			exit(0);
		}
		// network I/O 변화 있는 경우
		if (FD_ISSET(s, &read_fds)) {
			memset(buf, BUF_LEN, 0);
			if ((n = recv(s, buf, BUF_LEN, 0)) > 0) {
				do_recv(buf);
			}
			else {
				printf("recv error\n");
				break;
			}
		}
		// keyboard 입력이 있는 경우
		if (FD_ISSET(0, &read_fds)) {
			do_keyboard(s);
		}
	}
#endif
}

// 서버 message는 바로 출력만 한다.
void do_recv(char* buf)
{
	printf(buf);
}
// Keyboard에서 읽어서 서버로 전송하는 함수 (Linux/Windows 공용)
void do_keyboard(int s)
{
	char item;
	int amount;
	char buf1[BUF_LEN] = { 0 }, buf2[BUF_LEN] = { 0 };
	if (fgets(buf1, BUF_LEN, stdin) > 0) {
		sscanf(buf1, "%c %d", &item, &amount);
		item = toupper(item);
		memset(buf2, 0, BUF_LEN);
		if () { // item 이 X 이면 종료 처리
			// 004 전송하고 exit()
		}
		else { // 주문/완료 정보 전송.
			// mode가 1이면 001 item amount를 보내고
			printf("주문 데이터 전송\n");
			// mode가 2이면 002 item amout를 보낸다.
			printf("완료 데이타 전송\n");
		}
	}
	else {
		printf("fgets error\n");
		exit(0);
	}

}
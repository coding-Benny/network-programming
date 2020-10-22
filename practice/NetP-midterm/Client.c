/*
Client.c
2020�� ��Ʈ��ũ���α׷��� �߰���� �ǽ����� �����
chat_client1.c ���
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

	// winsock ����� ���� �ʼ�����
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
int mode; // 1: �ֹ� client, 2: �ֹ�

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
	// �������� �� ��.
	do {
		printf("Client ��� ���� (1: �ֹ�, 2: �ֹ�) : ");
		scanf("%d", &mode); getchar();
	} while (mode != 1 && mode != 2);

	/* ä�� ������ �����ּ� ����ü server_addr �ʱ�ȭ */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* �����û */
	printf("Client connecting %s %s\n", ip_addr, port_no);

	/* �����û */
	if (connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		exit(0);
	}
	else {
		printf("������ ���ӵǾ����ϴ�. \n");
	}
#ifdef WIN32
	u_long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode); // ������ non-blocking ���� �����.
	while (1) {
		memset(buf, 0, BUF_LEN);
		// Non-blocking read�̹Ƿ� �����Ͱ� ������ ��ٸ��� �ʰ� 0���� return
		n = recv(s, buf, BUF_LEN, 0);
		if (n > 0) { // non-blocking read
			do_recv(buf);
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK) {
			printf("recv error\n"); // server �� ����Ǿ��ų� ��Ʈ��ũ ����
			break;
		}
		if (kbhit()) { // key �� ���������� read key --> write to chat server
			do_keyboard(s);
		}
		Sleep(10); // Non-blocking I/O CPU �δ��� ���δ�.
	}
#else
	int maxfdp;
	fd_set read_fds;
	maxfdp = s + 1; // socket�� �׻� 0 ���� ũ�� �Ҵ�ȴ�.
	FD_ZERO(&read_fds);
	while (1) {
		memset(buf, 0, BUF_LEN);
		FD_SET(0, &read_fds); // stdin:0 ǥ���Է��� file ��ȣ = 0 �̴�.
		FD_SET(s, &read_fds); // server �� �߰�� socket ��ȣ

		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
			printf("select error\n");
			exit(0);
		}
		// network I/O ��ȭ �ִ� ���
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
		// keyboard �Է��� �ִ� ���
		if (FD_ISSET(0, &read_fds)) {
			do_keyboard(s);
		}
	}
#endif
}

// ���� message�� �ٷ� ��¸� �Ѵ�.
void do_recv(char* buf)
{
	printf(buf);
}
// Keyboard���� �о ������ �����ϴ� �Լ� (Linux/Windows ����)
void do_keyboard(int s)
{
	char item;
	int amount;
	char buf1[BUF_LEN] = { 0 }, buf2[BUF_LEN] = { 0 };
	if (fgets(buf1, BUF_LEN, stdin) > 0) {
		sscanf(buf1, "%c %d", &item, &amount);
		item = toupper(item);
		memset(buf2, 0, BUF_LEN);
		if () { // item �� X �̸� ���� ó��
			// 004 �����ϰ� exit()
		}
		else { // �ֹ�/�Ϸ� ���� ����.
			// mode�� 1�̸� 001 item amount�� ������
			printf("�ֹ� ������ ����\n");
			// mode�� 2�̸� 002 item amout�� ������.
			printf("�Ϸ� ����Ÿ ����\n");
		}
	}
	else {
		printf("fgets error\n");
		exit(0);
	}

}
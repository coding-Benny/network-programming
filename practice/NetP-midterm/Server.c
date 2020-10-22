/*
Server.c
2020�� ��Ʈ��ũ���α׷��� �߰���� �ǽ����� �����
chat_server1.c �� ����
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

#define MAXCLIENTS 64		// �ִ� ä�� ������ ��
#define EXIT	"004"		// ä�� ���� ���ڿ�
int maxfdp;              	// select() ���� �����ؾ��� # of socket ���� getmax() return �� + 1
int getmax(int);			// �ִ� ���Ϲ�ȣ ���
int num_chat = 0;         	// ä�� ������ ��
int client_fds[MAXCLIENTS];	// ä�ÿ� ������ ���Ϲ�ȣ ���
void RemoveClient(int);		// ä�� Ż�� ó�� �Լ�

#define BUF_LEN	128
#define CHAT_SERVER "0.0.0.0"
#define CHAT_PORT "30000"

#define N_MENU	2
int order[N_MENU] = { 0 }; // ��ü �ֹ��� ������
int clear[N_MENU] = { 0 }; // �Ϸᷮ ������
char* menus[N_MENU] = { "Pizza", "Chicken" };
// ��� ��Ȳ ���
void print_order()
{
	printf("*** �Ϸ�/�ֹ� ��Ȳ ***\n");
	for (int i = 0; i < N_MENU; i++) {
		printf(""); // ȭ��� ���� menus[], clear[], order[] ���� ���..
	}
}

// ��� ��Ȳ ����
int send_order(int j) {
	char buf[BUF_LEN] = { 0 };
	int ret;
	sprintf(buf, "*** �Ϸ�/�ֹ� ��Ȳ ***\n");
	ret = send(client_fds[j], buf, BUF_LEN, 0);
	if (ret <= 0) {
		printf("send error for client[%d]\n", j);
		return -1;
	}
	for (int i = 0; i < N_MENU; i++) {
		memset(buf, 0, BUF_LEN);
		sprintf(buf, ""); // ȭ��� ���� menus[], clear[], order[] ���� ����..
		// send() ó��
		// send()���� ���� ���� return -1
	}
	memset(buf, 0, BUF_LEN);
	sprintf(buf, "�ֹ�/�Ϸ� ������ �Է� (P)izza (C)Chiken + ���� (�� P 1) (����� X)\n");
	// send() ó��
	// send()���� �������� return -1
	return 1;
}

int main(int argc, char* argv[]) {
	char buf[BUF_LEN + 1] = { 0 };
	int i, j, n, ret;
	int server_fd, client_fd, client_len;
	unsigned int set = 1;
	char* ip_addr = CHAT_SERVER, * port_no = CHAT_PORT;
	fd_set  read_fds;     // �б⸦ ������ ���Ϲ�ȣ ����ü server_fd �� client_fds[] �� �����Ѵ�.
	struct sockaddr_in  client_addr, server_addr;
	int client_error[MAXCLIENTS];

#ifdef WIN32
	printf("Windows : ");
	init_winsock();
#else
	printf("Linux : ");
#endif
	/* ���� ���� */
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
#ifdef WIN32
	main_socket = server_fd;
#endif

	printf("chat_server1 waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));

	/* server_addr�� '\0'���� �ʱ�ȭ */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	/* server_addr ���� */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}
	/* Ŭ���̾�Ʈ�κ��� �����û�� ��ٸ� */
	listen(server_fd, 5);

	print_order();
	while (1) {
		FD_ZERO(&read_fds); // ���� �ʱ�ȭ
		FD_SET(server_fd, &read_fds); // accept() ��� ���� ����
		for (i = 0; i < num_chat; i++) // ä�ÿ� �������� ��� client ������ reac() ��� �߰�
			FD_SET(client_fds[i], &read_fds);
		maxfdp = getmax(server_fd) + 1;     // ���ô�� ������ ���� ���
		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) <= 0) {
			printf("select error <= 0 \n");
			exit(0);
		}
		// �ʱ� ���� ��, server_fd �� ��ȭ�� �ִ��� �˻�
		if (FD_ISSET(server_fd, &read_fds)) {
			// ��ȭ�� �ִ� --> client �� connect�� ���� ��û�� �� ��
			client_len = sizeof(client_addr);
			client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
			if (client_fd == -1) {
				printf("accept error\n");
			}
			else {
				printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr),
					ntohs(client_addr.sin_port));
				printf("client_fd = %d\n", client_fd);
				/* ä�� Ŭ���̾�Ʈ ��Ͽ� �߰� */
				printf("client[%d] ����. ���� ������ �� = %d\n", num_chat, num_chat + 1);
				client_fds[num_chat++] = client_fd;

				// �����ϸ� ��ü �ֹ� ��Ȳ�� �����ش�.
				// send_order(); ȣ��
			}
		}

		memset(client_error, 0, sizeof(client_error));
		/* Ŭ���̾�Ʈ�� ���� �޽����� ��� Ŭ���̾�Ʈ���� ��� */
		for (i = 0; i < num_chat; i++) {
			// ������ client���� I/O ��ȭ�� �ִ���.
			if (FD_ISSET(client_fds[i], &read_fds)) {
				// Read One �Ǵ� client ������ ���� Ȯ��
				memset(buf, 0, BUF_LEN);
				if ((n = recv(client_fds[i], buf, BUF_LEN, 0)) <= 0) {
					// client �� �� ���� ������ ���
					printf("recv error for client[%d]\n", i);
					client_error[i] = 1;
					continue;
				}
				printf("received %d from client[%d] : %s", n, i, buf);
				// ���Ṯ�� ó��
				if ("004") {
					RemoveClient(i);
					continue;
				}
				else if ("001") { // 001 �ֹ� ������ ó��
					// �ֹ� ������ ó��
					// 001 P 1
					char cmd[4];
					char menu;
					int amount;
					sscanf(buf, "%s %c %d", cmd, &menu, &amount);
					// P�� Pizza order[0]
					// C�� Chicken order[1] ����
					print_order();
					// ��� �����ڿ��� ��ȭ�� ��� ���
					// Wrie All]			
					for (int j;;) {
						// send_order() ���
						// 
					}
				}
				else if ("002") {
					// �Ϸ� ������ ó�� 
					// 002 C 1
					char cmd[4];
					char menu;
					int amount;
					sscanf(buf, "%s %c %d", cmd, &menu, &amount);
					// P�� Pizza clear[0]
					// C�� Chicken clear[1] ����
					print_order();
					// ��� �����ڿ��� ��ȭ�� ��� ���
					// Wrie All]			
					for (int j;;) {
						// send_order() ���
						// 
					}
				}

			}
		}
		// ������ �� Client���� �ڿ��� ������ ���鼭 �����Ѵ�.
		for (i = num_chat - 1; i >= 0; i--) {
			if (client_error[i])
				RemoveClient(i);
		}
	}
}

/* ä�� Ż�� ó�� */
void RemoveClient(int i) {
#ifdef WIN32
	closesocket(client_fds[i]);
#else
	close(client_fds[i]);
#endif
	// ������ client�� ������ �ڸ��� �̵� (��ĭ�� ���� �ʿ䰡 ����)
	if (i != num_chat - 1)
		client_fds[i] = client_fds[num_chat - 1];
	num_chat--;
	printf("client[%d] ����. ���� ������ �� = %d\n", i, num_chat);
}

// client_fds[] ���� �ִ� ���Ϲ�ȣ Ȯ��
// select(maxfds, ..) ���� maxfds = getmax(server_fd) + 1
int getmax(int k) {
	int max = k;
	int r;
	for (r = 0; r < num_chat; r++) {
		if (client_fds[r] > max) max = client_fds[r];
	}
	return max;
}
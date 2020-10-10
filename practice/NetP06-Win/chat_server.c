/*
 ���ϸ� : chat_server4.c
 ��  �� : ä�ü���, chat_server3 + /to, /sleep, /wakeup, /with, /withend, /sendfile, .. ��� �߰� ����
 ���� : chat_server4 [port]
*/
/*
	1:1 ��ȭ ������ �� �̸����κ��� �ε����� �˾Ƴ��� userwith[i]=j; userwith[j]=i; �� ����
	1:1 ��ȭ ������ �� �̸����κ��� �ε����� �˾Ƴ��� userwith[i]=NULL; userwith[j]=NULL; �� ����
*/
#ifdef _WIN32
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#else
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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
#define EXIT	"exit"		// ä�� ���� ���ڿ�
int maxfdp;              	// select() ���� �����ؾ��� # of socket ���� getmax() return �� + 1
int getmax(int);			// �ִ� ���Ϲ�ȣ ���
int num_chat = 0;         	// ä�� ������ ��
int client_fds[MAXCLIENTS];	// ä�ÿ� ������ ���Ϲ�ȣ ���
void RemoveClient(int);		// ä�� Ż�� ó�� �Լ�

#define BUF_LEN	128
#define CHAT_SERVER "0.0.0.0"
#define CHAT_PORT "30000"
char userlist[MAXCLIENTS][BUF_LEN]; // user name ������
int usersleep[MAXCLIENTS] = { 0 }; // sleep �������� ���� ����
int userwith[MAXCLIENTS];	// 1:1 ä�� ���� ���� ����
int k = -1;

#define CHAT_CMD_LOGIN		"/login"	// connect�ϸ� user name ���� "/login atom"
#define CHAT_CMD_LIST		"/list"		// userlist ��û
#define CHAT_CMD_EXIT		"/exit"		// ����
#define CHAT_CMD_TO			"/to"		// �ӼӸ� "/to atom Hi there.."
#define CHAT_CMD_SLEEP		"/sleep"	// �����(������) ����
#define CHAT_CMD_WAKEUP		"/wakeup"	// wakeup �Ǵ� message �����ϸ� �ڵ� wakeup
#define CHAT_CMD_WITH		"/with"		// /with nickname , nickname�� 1:1 ä�� ��� ����
#define CHAT_CMD_WITH_YES	"/withyes"		// 1:1 ��ȭ ��� [user2] /withyes user1
#define CHAT_CMD_WITH_NO	"/withno"		// 1:1 ��ȭ �ź� [user2] /withno user1
#define CHAT_CMD_WITH_END	"/end"		// 1:1 ä�� ���� [user1] /end or [user2] /end
#define CHAT_CMD_FILESEND	"/filesend"	// /filesend nickname data.txt ���� ����

int main(int argc, char* argv[]) {
	char buf[BUF_LEN], buf1[BUF_LEN], buf2[BUF_LEN], buf3[BUF_LEN];
	char recipient[BUF_LEN] = { 0 }, msg[BUF_LEN] = { 0 }, status[BUF_LEN] = { 0 };
	int i, j, n, ret;
	int server_fd, client_fd, client_len;
	unsigned int set = 1;
	char* ip_addr = CHAT_SERVER, * port_no = CHAT_PORT;
	fd_set  read_fds;     // �б⸦ ������ ���Ϲ�ȣ ����ü server_fd �� client_fds[] �� �����Ѵ�.
	struct sockaddr_in  client_addr, server_addr;
	int client_error[MAXCLIENTS];

	for (j = 0; j < BUF_LEN; j++) {
		userwith[j] = -1;
	}
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

	printf("chat_server5 waiting connection..\n");
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
			}
		}

		memset(client_error, 0, sizeof(client_error));
		/* Ŭ���̾�Ʈ�� ���� �޽����� ��� Ŭ���̾�Ʈ���� ��� */
		for (i = 0; i < num_chat; i++) {
			// ������ client���� I/O ��ȭ�� �ִ���.
			if (FD_ISSET(client_fds[i], &read_fds)) {
				// Read One �Ǵ� client ������ ���� Ȯ��
				if ((n = recv(client_fds[i], buf, BUF_LEN, 0)) <= 0) {
					// client �� �� ���� ������ ���
					printf("recv error for client[%d]\n", i);
					client_error[i] = 1;
					continue;
				}
				printf("received %d from client[%d] : %s", n, i, buf);
				// "/login username" --> buf1 = /login, buf2 = username
				// "/list" --> buf1 = /list
				// "[username] message .." buf1 = [username], buf2 = message ...
				sscanf(buf, "%s", buf1); // ó�� ���ڿ� �и� strtok() ������� �ʴ´�.
				n = strlen(buf1); // "/login username" or "[username] Hello" ���� /login �� [username] �� �и�
				strncpy(buf2, buf + n + 1, BUF_LEN - (n + 1)); // username �Ǵ� �޽��� �и�
				sscanf(buf2, "%s", buf2);
				strncpy(buf3, buf + n + strlen(buf2) + 2, BUF_LEN - (n + 1));

				/* �α��� */
				if (strncmp(buf1, CHAT_CMD_LOGIN, strlen(CHAT_CMD_LOGIN)) == 0) { // "/login"
					strcpy(userlist[i], buf2); // username ����
					printf("\nuserlist[%d] = %s\n", i, userlist[i]);
					for (j = 0; j < num_chat; j++) {
						if (j != i) {	// ���� ���� �ٸ� ����ڿ��� ������ �˸���.
							sprintf(buf, "[%s]���� �����Ͽ����ϴ�.\n", userlist[i]);
							ret = send(client_fds[j], buf, BUF_LEN, 0);
							if (ret <= 0) {
								printf("send error for client[%d]\n", j);
								client_error[j] = 1;
							}
						}
					}
					continue;
				}
				/* ������ ����Ʈ */
				if (strncmp(buf2, CHAT_CMD_LIST, strlen(CHAT_CMD_LIST)) == 0) { // "/list"
					printf("Sending user list to client[%d] %s\n", i, userlist[i]);
					sprintf(buf, "User List\nNo\tname\tstatus\n-----------------------\n");
					if (send(client_fds[i], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[i] = 1;
						continue;
					}
					for (j = 0; j < num_chat; j++) {
						sprintf(buf, "%02d\t%s\t%s\n", j, userlist[j], usersleep[j] ? "S" : "O");
						if (send(client_fds[i], buf, BUF_LEN, 0) < 0) {
							printf("client[%d] send error.", i);
							client_error[i] = 1;
							break;
						}
					}
					sprintf(buf, "-----------------------\n");
					if (send(client_fds[i], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[i] = 1;
						continue;
					}
					continue;
				}
				/* �ӼӸ� ��� */
				if (strncmp(buf2, CHAT_CMD_TO, strlen(CHAT_CMD_TO)) == 0) {	// "/to"
					char username[BUF_LEN], to[BUF_LEN], to_user[BUF_LEN], msg[BUF_LEN];
					sscanf(buf, "%s %s %s", username, to, to_user);
					strcpy(msg, buf + strlen(username) + strlen(to) + strlen(to_user) + 3);
					printf("[�ӼӸ�] from %s to %s : %s", username, to_user, msg);
					// �ӼӸ� ����
					for (j = 0; j < MAXCLIENTS; j++) {
						// user�� sleep�̸� � �޽����� �������� �ʴ´�.
						if (strcmp(userlist[j], to_user) == 0 && usersleep[j] != 1) {
							sprintf(buf2, "[�ӼӸ�] %s %s", username, msg);
							if (send(client_fds[j], buf, BUF_LEN, 0) < 0) {
								printf("send error for client[%d]\n", j);
								client_error[j] = 1;
								break;
							}
						}
					}
					continue;
				}
				/* ������ */
				if (strncmp(buf2, CHAT_CMD_SLEEP, strlen(CHAT_CMD_SLEEP)) == 0) {	// "/sleep"
					usersleep[i] = 1;
					continue;
				}
				/* ��� */
				if (strncmp(buf2, CHAT_CMD_WAKEUP, strlen(CHAT_CMD_WAKEUP)) == 0) {	// "/wakeup"
					usersleep[i] = 0;
					continue;
				}
				/* 1:1 ä�ø�� ���� */
				if (strncmp(buf2, CHAT_CMD_WITH_YES, strlen(CHAT_CMD_WITH_YES)) == 0) {
					for (j = 0; j < num_chat; j++) {
						if (strncmp(buf3, userlist[j], strlen(userlist[j])) == 0) {
							k = j;
							break;
						}
					}
					userwith[i] = k;
					userwith[k] = i;
					printf("%d:%d %d:%d\n", k, userwith[i], i, userwith[k]);
					sprintf(buf, "[%s]�� 1:1 ��ȭ ����\n", userlist[userwith[k]]);
					if (send(client_fds[userwith[i]], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[i] = 1;
						continue;
					}
					sprintf(buf, "[%s]�� 1:1 ��ȭ ����\n", userlist[userwith[i]]);
					if (send(client_fds[userwith[k]], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[k] = 1;
						continue;
					}
					break;
				}
				/* 1:1 ä�ø�� �ź� */
				if (strncmp(buf2, CHAT_CMD_WITH_NO, strlen(CHAT_CMD_WITH_NO)) == 0) {
					for (j = 0; j < num_chat; j++) {
						if (strncmp(buf3, userlist[j], strlen(userlist[j])) == 0) {
							sprintf(buf, "[%s]���� 1:1 ��ȭ�� �ź��߽��ϴ�.\n", userlist[i]);
							if (send(client_fds[j], buf, BUF_LEN, 0) < 0) {
								printf("client[%d] send error.", i);
								client_error[i] = 1;
								continue;
							}
							break;
						}
					}
					continue;
				}
				/* 1:1 ä�ø�� ��û */
				if (strncmp(buf2, CHAT_CMD_WITH, strlen(CHAT_CMD_WITH)) == 0) {
					char username[BUF_LEN], with[BUF_LEN], to_user[BUF_LEN];
					sscanf(buf, "%s %s %s", username, with, to_user);
					for (j = 0; j < MAXCLIENTS; j++) {	
						// user�� sleep�̸� � �޽����� �������� �ʴ´�.
						if (strcmp(userlist[j], to_user) == 0 && usersleep[j] != 1) {
							sprintf(buf2, "/with %s", userlist[i]);
							n = send(client_fds[j], buf2, BUF_LEN, 0);
							if (n < 0) {
								printf("send error for client[%d]\n", j);
								client_error[j] = 1;
								break;
							}
							printf("sending %d to client[%d] : %s\n", n, j, buf2);
							break;
						}
						continue;
					}
					continue;
				}
				/* 1:1 ä�ø�� ���� */
				if (strncmp(buf2, CHAT_CMD_WITH_END, strlen(CHAT_CMD_WITH_END)) == 0) {
					sprintf(buf, "[%s]�԰� 1:1 ��ȭ�� �����մϴ�.\n", userlist[i]);
					if (send(client_fds[userwith[i]], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[i] = 1;
						continue;
					}
					sprintf(buf, "[%s]�԰� 1:1 ��ȭ�� �����մϴ�.\n", userlist[userwith[i]]);
					if (send(client_fds[i], buf, BUF_LEN, 0) < 0) {
						printf("client[%d] send error.", i);
						client_error[k] = 1;
						continue;
					}
					userwith[userwith[i]] = -1;
					userwith[i] = -1;
					break;
				}
				/* ä�� ���� */
				if (strncmp(buf2, CHAT_CMD_EXIT, strlen(CHAT_CMD_EXIT)) == 0) { // "/exit"
					RemoveClient(i);
					continue;
				}

				usersleep[i] = 0;	// message�� ������ ������ �����.

				/* ��� ä�� �����ڿ��� �޽��� ��� */
				if (userwith[i] == -1) {
					for (j = 0; j < num_chat; j++) {
						if (usersleep[j] != 1 && userwith[j] == -1) {	// user�� sleep�̸� � �޽����� �������� �ʴ´�.
							ret = send(client_fds[j], buf, BUF_LEN, 0);
							if (ret <= 0) {
								printf("send error for client[%d]\n", j);
								client_error[j] = 1;
							}
						}
					}
				}
				else {
					if (usersleep[userwith[i]] != 1) {	// user�� sleep�̸� � �޽����� �������� �ʴ´�.
						ret = send(client_fds[userwith[i]], buf, BUF_LEN, 0);
						if (ret <= 0) {
							printf("send error for client[%d]\n", i);
							client_error[i] = 1;
						}
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
	printf("client[%d] %s ����. ���� ������ �� = %d\n", i, userlist[i], num_chat - 1);
	for (int j = 0; j < num_chat; j++) {
		char buf[BUF_LEN];
		if (j != i) {	// ���� ���� �ٸ� ����ڿ��� ������ �˸���.
			sprintf(buf, "[%s]���� �����Ͽ����ϴ�.\n", userlist[i]);
			send(client_fds[j], buf, BUF_LEN, 0);
		}
	}
	if (i != num_chat - 1) {
		client_fds[i] = client_fds[num_chat - 1];	// socket ����
		strcpy(userlist[i], userlist[num_chat - 1]);	// username
		usersleep[i] = usersleep[num_chat - 1];	// sleep ����
	}
	num_chat--;
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
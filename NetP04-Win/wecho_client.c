/*
 ���ϸ� : echo_client.c
 ��  �� : echo ���񽺸� �䱸�ϴ� TCP(������) Ŭ���̾�Ʈ
 ������ : cc -o echo_client echo_client.c
 ���� : echo_client [host] [port]
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

	// winsock ����� ���� �ʼ�����
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


	/* echo ������ �����ּ� ����ü �ۼ� */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* �����û */
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
	buf[n] = '\0'; // ���ڿ� ���� NULL �߰�
	printf("Received : %s\n", buf);

	while (1) {
		/* �α��� */
		printf("ID : ");
		scanf("%s", id);
		printf("Password : ");
		scanf("%s", pwd);
		
		sprintf(user_info, "id=%s pass=%s", id, pwd);

		/* echo ������ �޽��� �۽� */
		if (send(s, user_info, BUF_LEN, 0) < 0) {
			printf("send error\n");
			exit(0);
		}

		if ((n = recv(s, buf, BUF_LEN, 0)) < 0) {
			printf("recv error\n");
			exit(0);
		}
		buf[n] = '\0'; // ���ڿ� ���� NULL �߰�
		printf("%s", buf);
		if (strstr(buf, "Welcome") != NULL)
			break;
	}

	while (1) {
		printf("*** ��/�ҹ��� ��ȯ �޴��Դϴ�. ***\n");
		printf(" (1) ��� �빮�� ��ȯ\n");
		printf(" (2) ��� �ҹ��� ��ȯ\n");
		printf(" (3) ��>�� ��>�� ��ȯ\n");
		printf(" (4) ����\n");
		/* �޴� ���� */
		printf("�����ϼ��� : ");
		scanf("%s", req);
		while ((c = getchar()) != EOF && c != '\n');

		if (strcmp(req, "4") == 0) {
			if (send(s, req, BUF_LEN, 0) < 0) {
				printf("send error\n");
			}
			exit(0);
		}

		/* ���ڿ� �Է� */
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

		/* echo ������ �޽��� �۽� */
		if (send(s, req, BUF_LEN, 0) < 0) {
			printf("send error\n");
			exit(0);
		}

		if ((n = recv(s, buf, BUF_LEN, 0)) < 0) {
			printf("recv error\n");
			exit(0);
		}

		buf[n] = '\0'; // ���ڿ� ���� NULL �߰�
		printf("Received len=%d : %s\n", n, buf);
	}
	closesocket(s);
	return(0);
}
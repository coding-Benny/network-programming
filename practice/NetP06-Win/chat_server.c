/*----------------------
 파일명 : chat_server2.c
 기  능 : 채팅서버 username 과 exit 처리, select 함수 이용, (Windows 와 Linux 동일)
 사용법 : chat_server [port]
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

	// winsock 사용을 위해 필수적임
	signal(SIGINT, exit_callback);
	sversion = MAKEWORD(1, 1);
	WSAStartup(sversion, &wsadata);
}
#endif

#define MAXCLIENTS 64		// 최대 채팅 참가자 수
#define EXIT	"exit"		// 채팅 종료 문자열
int maxfdp;              	// select() 에서 감시해야할 # of socket 변수 getmax() return 값 + 1
int getmax(int);			// 최대 소켓번호 계산
int num_chat = 0;         	// 채팅 참가자 수
int client_fds[MAXCLIENTS];	// 채팅에 참가자 소켓번호 목록
void RemoveClient(int);		// 채팅 탈퇴 처리 함수

#define BUF_LEN	128
#define CHAT_SERVER "0.0.0.0"
#define CHAT_PORT "30000"
#define USE_NICKNAME

char username[BUF_LEN];

int main(int argc, char* argv[]) {
	char buf[BUF_LEN];
	int i, j, n, ret;
	int server_fd, client_fd, client_len;
	unsigned int set = 1;
	char* ip_addr = CHAT_SERVER, * port_no = CHAT_PORT;
	fd_set  read_fds;     // 읽기를 감지할 소켓번호 구조체 server_fd 와 client_fds[] 를 설정한다.
	struct sockaddr_in  client_addr, server_addr;
	int client_error[MAXCLIENTS];

#ifdef WIN32
	printf("Windows : ");
	init_winsock();
#else
	printf("Linux : ");
#endif
	/* 소켓 생성 */
	if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}
#ifdef WIN32
	main_socket = server_fd;
#endif

	printf("chat_server2 waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&set, sizeof(set));

	/* server_addr을 '\0'으로 초기화 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	/* server_addr 세팅 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}
	/* 클라이언트로부터 연결요청을 기다림 */
	listen(server_fd, 5);

	while (1) {
		FD_ZERO(&read_fds); // 변수 초기화
		FD_SET(server_fd, &read_fds); // accept() 대상 소켓 설정
		for (i = 0; i < num_chat; i++) // 채팅에 참가중이 모든 client 소켓을 reac() 대상 추가
			FD_SET(client_fds[i], &read_fds);
		maxfdp = getmax(server_fd) + 1;     // 감시대상 소켓의 수를 계산
		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) <= 0) {
			printf("select error <= 0 \n");
			exit(0);
		}
		// 초기 소켓 즉, server_fd 에 변화가 있는지 검사
		if (FD_ISSET(server_fd, &read_fds)) {
			// 변화가 있다 --> client 가 connect로 연결 요청을 한 것
			client_len = sizeof(client_addr);
			client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
			if (client_fd == -1) {
				printf("accept error\n");
			}
			else {
				printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr),
					ntohs(client_addr.sin_port));
				printf("client_fd = %d\n", client_fd);
				/* 채팅 클라이언트 목록에 추가 */
				printf("client[%d] 입장. 현재 참가자 수 = %d\n", num_chat, num_chat + 1);
				client_fds[num_chat++] = client_fd;
			}
		}

		memset(client_error, 0, sizeof(client_error));
		/* 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송 */
		for (i = 0; i < num_chat; i++) {
			// 각각의 client들의 I/O 변화가 있는지.
			if (FD_ISSET(client_fds[i], &read_fds)) {
				// Read One 또는 client 비정상 종료 확인
				if ((n = recv(client_fds[i], buf, BUF_LEN, 0)) <= 0) {
					// client 가 비 정상 종료한 경우
					printf("recv error for client[%d]\n", i);
					client_error[i] = 1;
					continue;
				}
				printf("received %d from client[%d] : %s", n, i, buf);
				sscanf(buf, "%s", username);
				printf("\nnickname=%s\n", username);

				// 종료문자 처리
				if (strncmp(buf + strlen(username) + 1, EXIT, strlen(EXIT)) == 0) {
					RemoveClient(i);
					continue;
				}
				// 모든 채팅 참가자에게 메시지 방송
				//printf("%s", buf);
				// Wrie All
				for (j = 0; j < num_chat; j++) {
					ret = send(client_fds[j], buf, BUF_LEN, 0);
					if (ret <= 0) {
						printf("send error for client[%d]\n", j);
						client_error[i] = 1;
					}
				}
			}
		}
		// 오류가 난 Client들을 뒤에서 앞으로 가면서 제거한다.
		for (i = num_chat - 1; i >= 0; i--) {
			if (client_error[i])
				RemoveClient(i);
		}
	}
}

/* 채팅 탈퇴 처리 */
void RemoveClient(int i) {
#ifdef WIN32
	closesocket(client_fds[i]);
#else
	close(client_fds[i]);
#endif
	// 마지막 client를 삭제된 자리로 이동 (한칸씩 내릴 필요가 없다)
	if (i != num_chat - 1)
		client_fds[i] = client_fds[num_chat - 1];
	num_chat--;
	printf("client[%d] 퇴장. 현재 참가자 수 = %d\n", i, num_chat);
}

// client_fds[] 내의 최대 소켓번호 확인
// select(maxfds, ..) 에서 maxfds = getmax(server_fd) + 1
int getmax(int k) {
	int max = k;
	int r;
	for (r = 0; r < num_chat; r++) {
		if (client_fds[r] > max) max = client_fds[r];
	}
	return max;
}
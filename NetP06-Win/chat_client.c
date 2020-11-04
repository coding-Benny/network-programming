/*
파일명 : chat_client6.c
기  능 : 채팅 클라이언트, username 사용, /login, /list, /exit /sleep /wakeup /to 처리, chat_clien3와 동일.
사용법 : chat_client4 [host] [port]
네트워크와 키보드 동시 처리 방법
Linux : select() 사용
Windows : socket()을 Non-blocking mode 와 kbhit()을 이용하여 폴링 구조 사용
*/
#ifdef _WIN32
#include <winsock.h>
#include <signal.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <errno.h>
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

#define CHAT_SERVER "127.0.0.1"
#define CHAT_PORT "30000"
#define BUF_LEN 128

#define CHAT_CMD_LOGIN	"/login"
#define CHAT_CMD_LIST	"/list"
#define CHAT_CMD_EXIT	"/exit"
#define CHAT_CMD_WITH		"/with"		// /with nickname , nickname과 1:1 채팅 모드 시작
#define CHAT_CMD_WITH_YES	"/withyes"	// 1:1 대화 허락 [user2] /withyes user1
#define CHAT_CMD_WITH_NO	"/withno"	// 1:1 대화 거부 [user2] /withno user1
#define CHAT_CMD_WITH_END	"/end"		// 1:1 채팅 종료 [user1] /end or [user2] /end
#define CHAT_CMD_FILE_SEND	"/filesend"		// [user1] /filesend user2 data.txt 파일 전송 요청
#define CHAT_CMD_FILE_YES	"/fileyes"		// [user2] /fileyes user1 파일 전송 허락
#define	CHAT_CMD_FILE_NO	"/fileno"		// [user2] /fileno user1 파일 수신 거부
#define CHAT_CMD_FILE_NAME	"/filename"		// [user2] /filename data.txt 765 파일 정보 전달
#define CHAT_CMD_FILE_DATA	"/filedata"		// [user1] /filedata data...
#define CHAT_CMD_FILE_END	"/fileend"		// [user1] /fileend 파일전송 끝

char username[BUF_LEN]; // user name
void read_key_send(int s, char* buf, char* buf2); // key입력후 보내는 code (Linux/Windows공용)

int main(int argc, char* argv[]) {
	char buf1[BUF_LEN + 1], buf2[BUF_LEN + 1], buf3[BUF_LEN + 1];
	char res[BUF_LEN], from_user[BUF_LEN], filename[BUF_LEN];
	int s, n, len_in, len_out, filesize = 0;
	struct sockaddr_in server_addr;
	char* ip_addr = CHAT_SERVER, * port_no = CHAT_PORT;
	FILE* fp;
	struct timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = 1000;

	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}
	printf("chat_client6-2 running.\n");
	printf("Enter user name : ");
	scanf("%s", username); getchar(); // \n제거

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

	/* 채팅 서버의 소켓주소 구조체 server_addr 초기화 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* 연결요청 */
	printf("Connecting %s %s\n", ip_addr, port_no);

	/* 연결요청 */
	if (connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("Client : Can't connect to server.\n");
		exit(0);
	}
	else {
		printf("채팅 서버에 접속되었습니다. \n");
	}
	memset(buf1, 0, BUF_LEN);
	sprintf(buf1, "%s %s", CHAT_CMD_LOGIN, username);
	if (send(s, buf1, BUF_LEN, 0) < 0) {
		printf("username send error\n");
		exit(0);
	}
#ifdef WIN32
	u_long iMode = 1;
	ioctlsocket(s, FIONBIO, &iMode); // 소켓을 non-blocking 으로 만든다.
	int maxfdp1;

	while (1) {
		// Non-blocking read이므로 데이터가 앖으면 기다리지 않고 0으로 return
		n = recv(s, buf2, BUF_LEN, 0);
		if (n > 0) { // non-blocking read
		// network에서 읽어서 화면에 출력
			if (strncmp(buf2, CHAT_CMD_WITH, strlen(CHAT_CMD_WITH)) == 0) {
				strncpy(buf3, buf2 + 6, BUF_LEN - (strlen(buf2) + 6));
				while (1) {
					printf("[%s]님이 1:1 대화를 요청했습니다.(y/n)? ", buf3);
					fgets(res, BUF_LEN, stdin);

					if (strcmp(res, "\n") == 0)
						continue;

					if (strcmp(res, "y\n") == 0)
						sprintf(res, "[%s] %s %s\n", username, CHAT_CMD_WITH_YES, buf3);
					else if (strcmp(res, "n\n") == 0)
						sprintf(res, "[%s] %s %s\n", username, CHAT_CMD_WITH_NO, buf3);

					if (send(s, res, BUF_LEN, 0) < 0) {
						printf("send error.\n");
						exit(0);
					}
					else
						break;
				}
				continue;
			}
			if (strncmp(buf2, CHAT_CMD_FILE_SEND, strlen(CHAT_CMD_FILE_SEND)) == 0) {
				char filesend[BUF_LEN];
				sscanf(buf2, "%s %s %s", filesend, from_user, filename);
				while (1) {
					printf("[%s]님이 %s 파일을 보내려고 합니다. 수신 (y/n)? ", from_user, filename);
					fgets(res, BUF_LEN, stdin);

					if (strcmp(res, "\n") == 0)
						continue;

					if (strcmp(res, "y\n") == 0)
						sprintf(res, "[%s] %s %s %s\n", username, CHAT_CMD_FILE_YES, from_user, filename);
					else if (strcmp(res, "n\n") == 0)
						sprintf(res, "[%s] %s %s\n", username, CHAT_CMD_FILE_NO, from_user);

					if (send(s, res, BUF_LEN, 0) < 0) {
						printf("send error.\n");
						exit(0);
					}
					else
						break;
				}
				continue;
			}
			if (strncmp(buf2, CHAT_CMD_FILE_NAME, strlen(CHAT_CMD_FILE_NAME)) == 0) {
				char command[BUF_LEN], filename2[BUF_LEN];
				sscanf(buf2, "%s %s %d", command, filename, &filesize);
				sprintf(filename2, "[%s]%s", from_user, filename);
				printf("[%s]님이 보내는 파일 %s %d bytes %s로 수신중...\n", from_user, filename, filesize, filename2);

				if ((fp = fopen(filename2, "wb")) == NULL) {
					printf("file open error\n");
					exit(0);
				}
				continue;
			}
			if (strncmp(buf2, CHAT_CMD_FILE_DATA, strlen(CHAT_CMD_FILE_DATA)) == 0) {
				char filedata[BUF_LEN], content[BUF_LEN];
				memset(content, 0, BUF_LEN + 1);
				sscanf(buf2, "%s %[^\t\n]", filedata, content);

				if (fwrite(content, n, 1, fp) <= 0) {
					printf("fwrite error\n");
					break;
				}
				continue;
			}
			if (strncmp(buf2, CHAT_CMD_FILE_YES, strlen(CHAT_CMD_FILE_YES)) == 0) {
				int readsum = 0, nread = 0;
				char fileyes[BUF_LEN], to_user[BUF_LEN];

				sscanf(buf2, "%s %s %s", fileyes, to_user, filename);
				if ((fp = fopen(filename, "rb")) == NULL) {
					printf("Can't open file %s\n", filename);
					exit(0);
				}

				fseek(fp, 0, 2);
				filesize = ftell(fp);
				rewind(fp);

				printf("[%s]님에게 파일 %s %d bytes 전송중...\n", to_user, filename, filesize);

				sprintf(res, "[%s] /filename %s %d\n", username, filename, filesize);
				if (send(s, res, BUF_LEN, 0) < 0) {
					printf("send error.\n");
					exit(0);
				}

				// send file contents
				readsum = 0;
				if (filesize < BUF_LEN)
					nread = filesize;
				else
					nread = BUF_LEN;

				while (readsum < filesize) {
					int n;
					memset(buf2, 0, BUF_LEN + 1);
					memset(buf3, 0, BUF_LEN + 1);
					n = fread(buf3, 1, BUF_LEN - (strlen(username) + 15), fp);	// read file

					if (n <= 0)	//  End of file ??
						break;

					sprintf(res, "[%s] /filedata %s\n", username, buf3);
					if (send(s, res, BUF_LEN, 0) <= 0) {	// only read bytes are sent to the network
						printf("send error\n");
						break;
					}
					readsum += n;
					if ((nread = (filesize - readsum)) > BUF_LEN)	// read remaining data
						nread = BUF_LEN;
				}
				fclose(fp);
				printf("%s %d bytes 전송 완료!\n", filename, filesize);

				sprintf(res, "[%s] /fileend\n", username);
				if (send(s, res, n, 0) <= 0) {	// only read bytes are sent to the network
					printf("send error\n");
					break;
				}
				continue;
			}
			if (strncmp(buf2, CHAT_CMD_FILE_END, strlen(CHAT_CMD_FILE_END)) == 0) {
				printf("%s님이 보낸 파일 %s %d bytes 수신 완료!\n", from_user, filename, filesize);
				fclose(fp);
				continue;
			}
			printf("%s", buf2);
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK) {
			printf("recv error\n"); // server 가 종료되었거나 네트워크 오류
			break;
		}
		if (kbhit()) { // key 가 눌려있으면 read key --> write to chat server
			read_key_send(s, buf1, buf2);
		}
		Sleep(100); // Non-blocking I/O CPU 부담을 줄인다.
	}
#else
	int maxfdp;
	fd_set read_fds;
	maxfdp = s + 1; // socket은 항상 0 보다 크게 할당된다.
	FD_ZERO(&read_fds);
	while (1) {
		FD_SET(0, &read_fds); // stdin:0 표준입력은 file 번호 = 0 이다.
		FD_SET(s, &read_fds); // server 와 견결된 socket 번호

		if (select(maxfdp, &read_fds, (fd_set*)0, (fd_set*)0, (struct timeval*)0) < 0) {
			printf("select error\n");
			exit(0);
		}
		// network I/O 변화 있는 경우
		if (FD_ISSET(s, &read_fds)) {
			if ((n = recv(s, buf2, BUF_LEN, 0)) > 0) {
				printf("%s", buf2);
			}
			else {
				printf("recv error\n");
				break;
			}
		}
		// keyboard 입력이 있는 경우
		if (FD_ISSET(0, &read_fds)) {
			read_key_send(s, buf1, buf2);
		}
	}
#endif
}

// Keyboard에서 읽어서 서버로 전송하는 함수 (Linux/Windows 공용)
void read_key_send(int s, char* buf1, char* buf2)
{
	printf("%s> ", username); // keyboard 입력이 있으면 앞에 prompt를 출력해준다.
	if (fgets(buf1, BUF_LEN, stdin) > 0) {
		sprintf(buf2, "[%s] %s", username, buf1);
		if (send(s, buf2, BUF_LEN, 0) < 0) {
			printf("send error.\n");
			exit(0);
		}
		if (strncmp(buf1, CHAT_CMD_EXIT, strlen(CHAT_CMD_EXIT)) == 0) {
			printf("Good bye.\n");
#ifdef WIN32
			closesocket(s);
#else
			close(s);
#endif
			exit(0);
		}
	}
	else {
		printf("fgets error\n");
		exit(0);
	}
}
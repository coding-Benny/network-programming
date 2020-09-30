/*
 파일명 : file_client1.c
 기  능 : echo_client1에서 Keyboard 대신 file 을 읽어서 전송
 컴파일 : cc -o file_client1 file_client1.c
 사용법 : file_client1 [host IP] [port]
*/
#ifdef WIN32
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

#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"
#define BUF_LEN 128

int main(int argc, char* argv[]) {
	int s, n, len_in, len_out;
	struct sockaddr_in server_addr;
	char* ip_addr = ECHO_SERVER, * port_no = ECHO_PORT;
	char buf[BUF_LEN + 1] = { 0 };


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



	/* echo 서버의 소켓주소 구조체 작성 */
	memset((char*)&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	// 파일명 입력
	FILE* fp;
	char filename[BUF_LEN] = "data.txt"; // data file 예
	printf("Enter file name : ");
	scanf("%s", filename);
	getchar(); // Enter key 처리.
	if ((fp = fopen(filename, "r")) == NULL) {
		printf("Can't open file %s\n", filename);
		exit(0);
	}

	/* 연결요청 */
	printf("Connecting %s %s\n", ip_addr, port_no);

	if (connect(s, (struct sockaddr*)&server_addr,
		sizeof(server_addr)) < 0) {
		printf("can't connect.\n");
		exit(0);
	}

	while (1) {
		/* 파일에서 입력을 받음 */
		//printf("Input string : ");
		//if (fgets(buf, BUF_LEN, stdin)) { // gets(buf);
		if (fgets(buf, BUF_LEN, fp)) {
			len_out = strlen(buf);
			buf[BUF_LEN] = '\0';
		}
		else {
			//printf("fgets error\n");
			printf("\nEnd of file\n");
			exit(0);
		}
		/* echo 서버로 메시지 송신 */
		printf("Sending len=%d : %s", len_out, buf);
		if (send(s, buf, len_out, 0) < 0) {
			printf("send error\n");
			exit(0);
		}
		if (strcmp(buf, "exit\n") == 0)
			break;
		if ((n = recv(s, buf, BUF_LEN, 0)) < 0) {
			printf("recv error\n");
			exit(0);
		}
		buf[n] = '\0'; // 문자열 끝에 NULL 추가
		printf("Received len=%d : %s", n, buf);
	}
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif
	return(0);
}



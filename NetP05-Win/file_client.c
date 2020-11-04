/*
 파일명 : file_client4.c
 기  능 : ftp 와 비슷하게 만들기. get, put, dir, quit 구현
 컴파일 : cc -o file_client4 file_client4.c
 사용법 : file_client4 [host IP] [port]
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

#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"
#define BUF_LEN 128

int main(int argc, char* argv[]) {
	int s, n, len_in, len_out;
	struct sockaddr_in server_addr;
	char* ip_addr = ECHO_SERVER, *port_no = ECHO_PORT;
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
	char filename[BUF_LEN] = { 0 }; // data file 예
	char command[BUF_LEN] = { 0 };
	char local_command[BUF_LEN] = { 0 };
	char req[BUF_LEN] = { 0 };

	/* 연결요청 */
	printf("Connecting %s %s\n", ip_addr, port_no);

	if (connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		printf("can't connect.\n");
		exit(0);
	}

	while (1) {
		int c, filesize = 0;
		int readsum = 0, nread;

		printf("file_client4> ");
		fgets(req, BUF_LEN, stdin);
		sscanf(req, "%s %s", command, filename);

		memset(buf, 0, BUF_LEN + 1);

		if (strcmp(command, "put") == 0) {
			if ((fp = fopen(filename, "rb")) == NULL) {
				printf("Can't open file %s\n", filename);
				exit(0);
			}

			fseek(fp, 0, 2);
			filesize = ftell(fp);
			rewind(fp);

			printf("Sending %s %d bytes.\n", filename, filesize);

			// send command, filename and filesize
			sprintf(buf, "%s %s %d", command, filename, filesize);
			if (send(s, buf, BUF_LEN, 0) <= 0) {	// transmission unit is BUF_LEN
				printf("command send error\n");
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
				// n = fgets(buf, BUF_LEN, fp);
				memset(buf, 0, BUF_LEN + 1);
				n = fread(buf, 1, BUF_LEN, fp);	// read file

				if (n <= 0)	//  End of file ??
					break;

				if (send(s, buf, n, 0) <= 0) {	// only read bytes are sent to the network
					printf("send error\n");
					break;
				}
				readsum += n;
				if ((nread = (filesize - readsum)) > BUF_LEN)	// read remaining data
					nread = BUF_LEN;
			}
			printf("File %s %d bytes transferred.\n\n", filename, filesize);
		}
		else if (strcmp(command, "get") == 0) {
			// send command, filename
			sprintf(buf, "%s %s", command, filename);
			if (send(s, buf, BUF_LEN, 0) <= 0) {	// transmission unit is BUF_LEN
				printf("command send error\n");
				exit(0);
			}
			memset(buf, 0, BUF_LEN + 1);
			// 2. 서버가 파일 사이즈를 알려주면
			if (recv(s, buf, BUF_LEN, 0) <= 0) {
				printf("filename recv error\n");
				exit(0);
			}
			sscanf(buf, "%s %d", filename, &filesize);

			if ((fp = fopen(filename, "wb")) == NULL) {
				printf("file open error\n");
				exit(0);
			}
			printf("Receving %s %d bytes.\n", filename, filesize);

			readsum = 0;
			if (filesize < BUF_LEN)
				nread = filesize;
			else
				nread = BUF_LEN;

			memset(buf, 0, BUF_LEN + 1);

			while (readsum < filesize) {
				n = recv(s, buf, nread, 0);
				if (n <= 0) {
					printf("\nend of file\n");
					break;
				}

				if (fwrite(buf, n, 1, fp) <= 0) {
					printf("fwrite error\n");
					break;
				}
				readsum += n;
				if ((nread = (filesize - readsum)) > BUF_LEN)
					nread = BUF_LEN;
			}
			printf("\nFile %s %d bytes received.\n\n", filename, filesize);
			fclose(fp);
		}
		else if (strcmp(command, "dir") == 0) {
			// send command
			sprintf(buf, "%s", command);
			if (send(s, buf, BUF_LEN, 0) <= 0) {	// transmission unit is BUF_LEN
				printf("command send error\n");
				exit(0);
			}
			// 명령어를 실행한 결과를 파일처럼 읽어서 보내준다.			
			while (1) {
				memset(buf, 0, 128);
				//fread(buf, 1, BUF_LEN, fp);
				recv(s, buf, 128, 0);

				if (strncmp(buf, "-EOF-", 5) == 0)
					break;
				
				printf(buf);
			}
			printf("\n");
		}
		else if (strcmp(command, "ldir") == 0) {
			// send command
			sprintf(buf, "%s", command);
			if (send(s, buf, BUF_LEN, 0) <= 0) {	// transmission unit is BUF_LEN
				printf("command send error\n");
				exit(0);
			}
#ifdef WIN32
			system("dir");
#else
			system("ls -l");
#endif
			printf("\n");
		}
		else if (command[0] == '!') {
			strcpy(local_command, command);

			// send command
			strcpy(buf, local_command);
			
			if (send(s, buf, BUF_LEN, 0) <= 0) {	// transmission unit is BUF_LEN
				printf("command send error\n");
				exit(0);
			}
			
			system(local_command + 1);
			printf("\n");
		}
		else if (strcmp(command, "quit") == 0) {
			printf("Client end.\n");
			if (send(s, command, BUF_LEN, 0) < 0)
				printf("send error\n");
			exit(0);
		}
		else {
			printf("Please enter the correct command..\n");
		}
	}
#ifdef WIN32
	closesocket(s);
#else
	close(s);
#endif
	return(0);
}

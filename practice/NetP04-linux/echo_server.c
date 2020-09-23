/*
파일명 : echo_server.c 
기  능 : echo 서비스를 수행하는 서버
컴파일 : cc -o echo_server echo_server.c
사용법 : echo_server [port]
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_LEN 128
#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"

int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;			/* 소켓번호 */
	int len, msg_size;
	char buf[BUF_LEN+1];
	unsigned int set = 1;
	char *ip_addr = ECHO_SERVER, *port_no = ECHO_PORT;

/*
	unsigned short s1 = 0x1234;
	unsigned short s2 = htons(s1);
	unsigned char* p = (unsigned char*)&s1;
	printf("%02x %02x\n",  p[1], p[0]);
	p = (unsigned char*)&s2;
	printf("%02x %02x\n",  p[1], p[0]);
*/
	
	if (argc == 2) {
		port_no = argv[1];
	}
	/* 소켓 생성 */
	if((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}

	printf("echo_server waiting connection..\n");
	printf("server_fd = %d\n", server_fd);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&set, sizeof(set));

	/* server_addr을 '\0'으로 초기화 */
	memset ((char *)&server_addr, 0, sizeof(server_addr));
	/* server_addr 세팅 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(port_no));

	/* bind() 호출 */
	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}

	/* 소켓을 수동 대기모드로 세팅 */
	listen(server_fd, 5);

	/* iterative  echo 서비스 수행 */
	printf("Server : waiting connection request.\n");
	len = sizeof(client_addr);

	while (1) {
		/* 연결요청을 기다림 */
		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
		if(client_fd < 0) {
			printf("Server: accept failed.\n");
			exit(0);
		}

		printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		printf("client_fd = %d\n", client_fd);
		msg_size = recv(client_fd, buf, BUF_LEN, 0);
		buf[msg_size] = '\0';
		printf("Received len=%d : %s", msg_size, buf);
		send(client_fd, buf, msg_size, 0);
		printf("Sending len=%d : %s", msg_size, buf);
		close(client_fd);
	}
	close(server_fd);
	return(0);
}



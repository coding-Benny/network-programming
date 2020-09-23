/*
 파일명 : echo_client.c 
 기  능 : echo 서비스를 요구하는 TCP(연결형) 클라이언트
 컴파일 : cc -o echo_client echo_client.c
 사용법 : echo_client [host] [port]
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <unistd.h>
#define ECHO_SERVER "127.0.0.1"
#define ECHO_PORT "30000"
#define BUF_LEN 128
int main(int argc, char *argv[]) {
	int s, n, len_in, len_out;
	struct sockaddr_in server_addr;
	char *ip_addr = ECHO_SERVER, *port_no = ECHO_PORT;
	char buf[BUF_LEN+1] = {0};

	if(argc == 3 ) {
		ip_addr = argv[1];
		port_no = argv[2];
	}

	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("can't create socket\n");
		exit(0);
	}
	/* echo 서버의 소켓주소 구조체 작성 */
	memset((char *)&server_addr, 0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip_addr);
	server_addr.sin_port = htons(atoi(port_no));

	/* 연결요청 */
	printf("Connecting %s %s\n", ip_addr, port_no);

	if(connect(s, (struct sockaddr *)&server_addr, 
	    sizeof(server_addr)) < 0) {
		printf("can't connect.\n");
		exit(0);
	}
	/* 키보드 입력을 받음 */
	printf("Input string : ");
	if (fgets(buf, BUF_LEN, stdin)) {
		len_out = strlen(buf);
		buf[BUF_LEN] = '\0';
	} else {
		printf("fgets error\n");
		exit(0);
	}
	/* echo 서버로 메시지 송신 */
	printf("Sending len=%d : %s", len_out,  buf);
	if (send(s, buf, len_out, 0) < 0) {
		printf("write error\n");
		exit(0);
	}
	
	if((n = recv(s, buf, BUF_LEN, 0)) < 0) {
			printf("read error\n");
			exit(0);
	}
	buf[n] = '\0'; // 문자열 끝에 NULL 추가
	printf("Received len=%d : %s", n,  buf);
	close(s);
	return(0);
}


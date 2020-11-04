/*
 파일명 : udp_time_client.c
 기  능 : time 서비스를 요구하는 UDP(비연결형) 클라이언트
 컴파일 : cc -w -o udp_time_client time_client.c
 사용법 : udp_time_client
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <unistd.h>

#define TIME_SERVER "127.0.0.1"
#define TIME_PORT "30000"

void main(int argc, char *argv[]) {
	int sock;
	struct sockaddr_in server;
	int buf_len, server_len;
	char buf[256] = "?";
	char *ip_addr = TIME_SERVER, *port_no = TIME_PORT;
	if (argc == 3) {
		ip_addr = argv[1];
		port_no = argv[2];
	}

	if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("can't create socket\n");
		exit(0);
	}

	/* time 서버의 소켓주소 구조체 작성 */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip_addr);
	server.sin_port = htons(atoi(port_no));
	buf[0] = '?'; buf[1] = '\0';
	buf_len = sendto(sock, buf, strlen(buf)+1, 0, (struct sockaddr *)&server, sizeof(server));
	buf_len = recvfrom(sock, buf, 256, 0, (struct  sockaddr *)&server, &server_len);
	if (buf_len < 0) {
		printf("recvfrom failed\n");
		exit(1);
	}
	printf("Time information from server is %s", buf);
	return(0);
}

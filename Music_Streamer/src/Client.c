#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"

#define STREAMING_PORT 50000

int client(SETTINGS *setUp)
{
	int retval;

	//윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	//서버와 통신할 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		err_quit("socket()");

	//서버의 IP, Port 설정
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(setUp->server_ip);
	serveraddr.sin_port = htons(STREAMING_PORT);

	//서버에 접속하기
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");
	else
		printf("서버와 연결되었습니다. \n\n");

	//----------------------------------------------------------------------

	//재생목록을 만든다. 재생목록 배열은 100 * 512이다.
	//재생목록 배열에서 0번 행은 쓰지 않는다. 따라서 총 99개의 재생목록을 저장할 수 있다.
	//재생목록에서 안쓰는 부분은 반드시 Null값으로 초기화한다.
	char playList[100][512] = { "\0" };

	retval = recvFullPlayList(sock, playList);
	if (retval != 0)
		printf("전체 재생목록 수신 오류. \n");
	//----------------------------------------------------------------------

	//서버와 연결을 종료한다.
	closesocket(sock);

	printf("프로그램을 종료합니다. \n");
	WSACleanup();
	return 0;
}
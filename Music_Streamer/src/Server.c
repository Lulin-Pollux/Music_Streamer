#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"

#define STREAMING_PORT 50000

char playList[50][256];  //재생목록

//재생목록을 초기화하는 함수
int initializePlaylist(char playlist[][512])
{
	strcpy_s(playlist[1], 512, "./playQue/1.mp3");
	strcpy_s(playlist[2], 512, "./playQue/2.mp3");
	strcpy_s(playlist[3], 512, "./playQue/3.mp3");
	strcpy_s(playlist[4], 512, "./playQue/4.mp3");

	return 0;
}

//서버 메인 함수
int server()
{
	int retval;

	//윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	//대기용 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");

	//지역 IP, Port 결정
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(STREAMING_PORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	//대기용 소켓을 listen상태로 변경
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");

	//통신용 소켓, 소켓 주소 구조체 생성
	SOCKET client_sock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);

	while (1)
	{
		//클라이언트의 접속을 기다림
		printf("클라이언트의 접속을 기다리는 중... \n\n");
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
			err_display("accept()");

		//접속한 클라이언트의 정보 출력
		printf("클라이언트가 접속하였습니다. \n");
		printf("주소= %s:%d \n\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		//----------------------------------------------------------------------

		//재생목록을 만든다. 재생목록 배열은 100 * 512이다.
		//재생목록 배열에서 0번 행은 쓰지 않는다. 따라서 총 99개의 재생목록을 저장할 수 있다.
		//재생목록에서 안쓰는 부분은 반드시 Null값으로 초기화한다.
		char playlist[100][512] = { "\0" };

		//재생목록을 초기화한다.
		retval = initializePlaylist(playlist);
		if (retval != 0)
		{
			printf("재생목록 초기화 실패. \n");
			break;
		}

		//재생목록에 있는 모든 파일을 전송한다.
		retval = sendFullPlaylist(client_sock, playlist);
		if (retval != 0)
		{
			printf("전체 재생목록 전송 오류. \n");
			break;
		}
		//----------------------------------------------------------------------

		//연결 종료
		closesocket(client_sock);
		break;
	}


	printf("서버를 종료합니다. \n");
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
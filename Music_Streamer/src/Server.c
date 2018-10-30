﻿#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"


//전체 재생목록을 출력한다.
int printFullPlaylist(char playlist[][512])
{
	printf("\t 재생목록 \n");
	printf("--------------------------------\n");
	for (int i = 1; i < 100; i++)
	{
		if (strlen(playlist[i]) == 0)
			break;
		else
		{
			char *pos = strrchr(playlist[i], '/');
			printf("%d. %s\n", i, pos + 1);
		}
	}

	return 0;
}

//서버 메인 함수
int server(SETTINGS sets)
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
	serveraddr.sin_port = htons(sets.server_mainPort);
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
		printf("\n클라이언트의 접속을 기다리는 중... \n");
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
			err_quit("accept()");

		//클라이언트가 접속할 경우, 클라이언트의 아이디와 닉네임을 수신한다.
		retval = recv(client_sock, (char*)&sets.client_uid, sizeof(sets.client_uid), MSG_WAITALL);
		if (retval == SOCKET_ERROR)
		{
			err_display("클라이언트 아이디recv()");
			closesocket(client_sock);
			continue;
		}
		retval = recv(client_sock, (char*)&sets.client_nickName, sizeof(sets.client_nickName), MSG_WAITALL);
		if (retval == SOCKET_ERROR)
		{
			err_display("클라이언트 닉네임recv()");
			closesocket(client_sock);
			continue;
		}
		
		//곧바로 서버의 아이디와 닉네임을 송신한다.
		retval = send(client_sock, (char*)&sets.server_uid, sizeof(sets.server_uid), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("서버 아이디send()");
			closesocket(client_sock);
			continue;
		}
		retval = send(client_sock, (char*)&sets.server_nickName, sizeof(sets.server_nickName), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("서버 닉네임send()");
			closesocket(client_sock);
			continue;
		}

		//접속한 클라이언트의 정보 출력
		printf("클라이언트가 접속하였습니다. \n");
		printf("아이디: %d, 닉네임: %s \n\n", sets.client_uid, sets.client_nickName);

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

		//전체 재생목록을 출력한다.
		printFullPlaylist(playlist);

		//재생목록에 있는 모든 파일을 전송한다.
		double allSendBytes = 0.0;
		retval = sendFullPlaylist(client_sock, playlist, &allSendBytes);
		if (retval != 0)
		{
			printf("전체 재생목록 전송 오류. \n");
			break;
		}
		else
			printf("전체 재생목록 전송 완료! (%0.2lfMB) \n", allSendBytes / 1024 / 1024);
		//----------------------------------------------------------------------


		//연결 종료
		closesocket(client_sock);
	}


	printf("서버를 종료합니다. \n");
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
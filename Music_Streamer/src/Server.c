#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"


//서버에서 서버와 클라이언트의 아이디, 닉네임을 교환하는 함수
int server_exchangeIdNickname(SOCKET sock, SETTINGS *sets)
{
	int retval;

	//클라이언트의 아이디와 닉네임을 수신한다.
	retval = recv(sock, (char*)&sets->client_uid, sizeof(sets->client_uid), MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("클라이언트 아이디recv()");
		return 1;
	}
	retval = recv(sock, (char*)&sets->client_nickName, sizeof(sets->client_nickName), MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("클라이언트 닉네임recv()");
		return 1;
	}

	//곧바로 서버의 아이디와 닉네임을 송신한다.
	retval = send(sock, (char*)&sets->server_uid, sizeof(sets->server_uid), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("서버 아이디send()");
		return 1;
	}
	retval = send(sock, (char*)&sets->server_nickName, sizeof(sets->server_nickName), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("서버 닉네임send()");
		return 1;
	}

	return 0;
}

//재생목록을 관리하는 스레드 
DWORD WINAPI operatePlaylist(LPVOID playlist)
{
	int retval;

	//재생목록을 초기화한다.
	retval = initializePlaylist(playlist);
	if (retval != 0)
	{
		textcolor(YELLOW);
		printf("재생목록 초기화 실패. \n");
		textcolor(RESET);
		return 1;
	}

	while (1)
	{
		int input = -1;
		printf("재생목록 검색(1), 재생목록 추가(2), 재생목록 삭제(3): ");
		scanf_s("%d", &input);
		clearInputBuffer();

		switch (input)
		{
		case 1:
			printFullPlaylist(playlist);
			break;
		case 2:
			printf("재생목록에 추가할 파일명을 입력해주세요. \n");
			printf(">>> ");
			char fileName[256];
			gets_s(fileName, sizeof(fileName));
			retval = addPlaylist(fileName, playlist);
			if (retval == 0)
				printf("재생목록을 추가했습니다.");
			break;
		case 3:
			printf("아직 개발 중입니다. \n");
			break;
		case 0:
			system("cls");
			break;
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

	//-----------------------------------------------------------------------
	//재생목록을 만든다. 재생목록 배열은 100 * 512이다.
	//재생목록 배열에서 0번 행은 쓰지 않는다. 따라서 총 99개의 재생목록을 저장할 수 있다.
	//재생목록에서 안쓰는 부분은 반드시 Null값으로 초기화한다.
	char playlist[100][512] = { "\0" };

	//재생목록 운영 스레드 생성
	HANDLE hThread1 = CreateThread(NULL, 0, operatePlaylist, playlist, 0, NULL);
	CloseHandle(hThread1);

	//서버 동작 시작
	while (1)
	{
		//클라이언트의 접속을 기다림
		printf("\n클라이언트의 접속을 기다리는 중... \n");
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
			err_quit("accept()");

		//클라이언트가 접속할 경우,
		//서버와 클라이언트의 아이디, 닉네임을 교환한다.
		retval = server_exchangeIdNickname(client_sock, &sets);
		if (retval != 0)
		{
			closesocket(client_sock);
			continue;
		}

		//접속한 클라이언트의 정보 출력
		textcolor(GREEN);
		printf("\n클라이언트가 접속하였습니다. \n");
		textcolor(WHITE);
		printf("아이디: %d, 닉네임: %s \n\n", sets.client_uid, sets.client_nickName);
		textcolor(RESET);

		//재생목록에 있는 모든 파일을 전송한다.
		double allSendBytes = 0.0;
		retval = sendFullPlaylist(client_sock, playlist, &allSendBytes);
		if (retval != 0)
		{
			printf("전체 재생목록 전송 오류. \n");
			break;
		}
		else
		{
			textcolor(GREEN);
			printf("전체 재생목록 전송 완료! (%0.2lfMB) \n", allSendBytes / 1024 / 1024);
			textcolor(RESET);
		}
	}


	printf("서버를 종료합니다. \n");
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
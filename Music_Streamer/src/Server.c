#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"

//스레드 함수의 매개변수용 구조체
struct clientCommParms
{
	SETTINGS sets;
	SOCKET sock;
	PLAYLIST_PTR playlist;
};


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

//서버를 관리하는 스레드 함수
DWORD WINAPI operateServerSystem(LPVOID playlist)
{
	int retval;

	//사용법 출력
	printf("\n");
	printf("재생목록: 1(출력), 2(추가), 3(변경), 4(삭제) \n");
	printf("자세한 설명은 매뉴얼을 참고하세요. \n");

	//명령을 입력받아 처리한다.
	while (1)
	{
		printf("\n");
		int input;
		scanf_s("%d", &input);
		clearInputBuffer();

		switch (input)
		{
		//재생목록 출력
		case 1:
			printFullPlaylist(playlist);
			break;
		//재생목록 추가
		case 2:
			printf("재생목록에 추가할 파일명을 입력해주세요. \n");
			printf(">>> ");
			char fileName[256];
			gets_s(fileName, sizeof(fileName));
			retval = insertPlaylist(fileName, playlist);
			if (retval == 0)
			{
				textcolor(GREEN);
				printf("재생목록을 추가했습니다. \n");
				textcolor(RESET);
			}
			break;
		//재생목록 변경
		case 3:
			retval = updatePlaylist(playlist);
			if (retval == 0)
			{
				textcolor(GREEN);
				printf("재생목록을 변경했습니다. \n");
				textcolor(RESET);
			}
			break;
		//재생목록 삭제
		case 4:
			printf("재생목록에서 삭제할 행을 입력해주세요. \n");
			printf(">>> ");
			int row;
			scanf_s("%d", &row);
			clearInputBuffer();
			retval = deletePlaylist(row, playlist);
			if (retval == 0)
			{
				textcolor(GREEN);
				printf("%d행을 삭제했습니다. \n", row);
				textcolor(RESET);
			}
			break;
		//화면 리셋
		case 0:
			system("cls");
			setScreenBufferSize(80, 9000);
			printf("\n");
			printf("재생목록: 1(출력), 2(추가), 3(변경), 4(삭제) \n");
			printf("자세한 설명은 매뉴얼을 참고하세요. \n");
			break;

		//오류제어
		default:
			printf("알 수 없는 입력입니다. \n");
			break;
		}
	}

	return 0;
}

//클라이언트와 각각 통신하는 스레드 함수
DWORD WINAPI clientComm(LPVOID arg)
{
	int retval;
	char buffer[512];

	//필요한 변수 선언
	SETTINGS sets = ((struct clientCommParms *)arg)->sets;
	SOCKET sock = ((struct clientCommParms *)arg)->sock;
	PLAYLIST_PTR playlist = ((struct clientCommParms *)arg)->playlist;

	//클라이언트가 접속할 경우,
	//서버와 클라이언트의 아이디, 닉네임을 교환한다.
	retval = server_exchangeIdNickname(sock, &sets);
	if (retval != 0)
	{
		textcolor(YELLOW);
		printf("클라이언트와 아이디, 닉네임을 교환하는데 실패했습니다. \n");
		textcolor(RESET);
		closesocket(sock);
		return 1;
	}

	//접속한 클라이언트의 정보 출력
	textcolor(GREEN);
	printf("\n클라이언트가 접속하였습니다. \n");
	textcolor(WHITE);
	printf("아이디: %d, 닉네임: %s \n\n", sets.client_uid, sets.client_nickName);
	textcolor(RESET);

	//재생목록에 있는 모든 파일을 전송한다.
	printf("전체 재생목록 전송 중... \n");
	double allSendBytes = 0.0;
	retval = sendFullPlaylist(sock, playlist, &allSendBytes);
	if (retval != 0)
	{
		printf("전체 재생목록 전송 오류. \n");
		closesocket(sock);
	}
	else
	{
		textcolor(GREEN);
		printf("전체 재생목록 전송 완료! (%0.2lfMB) \n", allSendBytes / 1024 / 1024);
		textcolor(RESET);
	}

	//클라이언트에서 보내는 요청을 처리한다.
	char request[50];
	while (1)
	{
		//요청 메시지 수신
		retval = recv(sock, request, sizeof(request), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("클라이언트 요청recv()");
			break;
		}

		//재생목록 새로고침 요청 처리
		if (strcmp(request, "refresh playlist") == 0)
		{
			//재생목록을 전송한다. (51200바이트 고정 길이)
			retval = send(sock, playlist[0], 51200, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("재생목록 send()");
				return 1;
			}
		}
		else if (strcmp(request, "download playlist") == 0)
		{
			//다운로드를 원하는 재생목록을 수신
			recv(sock, buffer, 512, MSG_WAITALL);
			
			//해당 파일을 전송한다.
			double sendBytes;
			sendFile(sock, buffer, &sendBytes);
		}
	}

	closesocket(sock);
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

	//소켓 옵션 설정
	BOOL bEnable = TRUE;
	retval = setsockopt(listen_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&bEnable, sizeof(bEnable));
	if (retval == SOCKET_ERROR)
		err_quit("KeepAlive 소켓 옵션setsockopt()");

	//IP, Port 재사용 설정
	BOOL optval = TRUE;
	retval = setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
	if (retval == SOCKET_ERROR)
		err_quit("ReUseAddr 소켓 옵션setsockopt()");

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

	//재생목록을 초기화한다.
	retval = initializePlaylist(playlist);
	if (retval != 0)
	{
		textcolor(YELLOW);
		printf("재생목록 초기화 실패. \n");
		textcolor(RESET);
		return 1;
	}

	//서버를 관리하는 스레드 함수
	HANDLE hThread1 = CreateThread(NULL, 0, operateServerSystem, playlist, 0, NULL);
	if (hThread1 == NULL)
	{
		printf("스레드 생성 실패");
		return 1;
	}
	else
		CloseHandle(hThread1);

	//서버가 작동됨을 알린다.
	textcolor(SKY_BLUE);
	printf("Music Streamer 서비스를 시작합니다. \n");
	textcolor(RESET);

	//서버 동작 시작
	while (1)
	{
		//클라이언트의 접속을 기다림
		client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
			err_quit("accept()");

		//구조체 변수에 매개변수를 복사
		struct clientCommParms parms;
		parms.sets = sets;
		parms.sock = client_sock;
		parms.playlist = playlist;

		//스레드 생성
		HANDLE hThread1 = CreateThread(NULL, 0, clientComm, &parms, 0, NULL);
		if (hThread1 == NULL)
		{
			printf("스레드 생성 실패");
			closesocket(client_sock);
		}
		else
			CloseHandle(hThread1);
	}


	printf("Music Streamer 서비스를 종료합니다. \n");
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
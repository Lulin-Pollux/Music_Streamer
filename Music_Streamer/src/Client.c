#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment (lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"


//서버에서 서버와 클라이언트의 아이디, 닉네임을 교환하는 함수
int client_exchangeIdNickname(SOCKET sock, SETTINGS *sets)
{
	int retval;

	//클라이언트의 아이디와 닉네임을 송신한다.
	retval = send(sock, (char*)&sets->client_uid, sizeof(sets->client_uid), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("클라이언트 아이디send()");
		return 1;
	}
	retval = send(sock, (char*)&sets->client_nickName, sizeof(sets->client_nickName), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("클라이언트 닉네임send()");
		return 1;
	}

	//곧바로 서버의 아이디와 닉네임을 수신한다.
	retval = recv(sock, (char*)&sets->server_uid, sizeof(sets->server_uid), MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("클라이언트 아이디recv()");
		return 1;
	}
	retval = recv(sock, (char*)&sets->server_nickName, sizeof(sets->server_nickName), MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("클라이언트 닉네임recv()");
		return 1;
	}

	return 0;
}

//재생목록을 새로고침하는 함수
int refreshPlaylist(SOCKET sock, char playlist[][512])
{
	int retval;

	//서버에게 재생목록 새로고침을 요청한다.
	char request[50] = { "refresh playlist" };
	retval = send(sock, request, (int)strlen(request) + 1, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("재생목록 새로고침 요청send()");
		return 1;
	}

	//서버로부터 재생목록을 다시 받아온다.
	retval = recv(sock, playlist[0], 51200, MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("재생목록 recv()");
		return 1;
	}

	//재생목록에서 없는 파일은 서버로부터 받아온다.
	for (int i = 1; i < 100; i++)
	{
		//i번째 재생목록이 비었으면 반복종료
		if (strlen(playlist[i]) == 0)
			break;

		//i번째 재생목록의 파일이 없는지 검사
		FILE *rfp;
		retval = fopen_s(&rfp, playlist[1], "r");
		if (retval != 0)
		{
			//재생목록 다운로드 요청
			strcpy_s(request, sizeof(request), "download playlist");
			send(sock, request, (int)strlen(request) + 1, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("재생목록 다운로드 요청send()");
				return 1;
			}

			//다운로드 받을 재생목록 전송
			send(sock, playlist[i], 512, 0);
			if (retval == SOCKET_ERROR)
			{
				err_display("다운로드 받을 재생목록send()");
				return 1;
			}

			//파일을 다운로드 받음
			double recvBytes;
			retval = recvFile(sock, playlist[i], &recvBytes);
			if (retval != 0)
				return 1;
		}
		else
			fclose(rfp);
	}

	return 0;
}

//클라이언트 메인 함수
int client(SETTINGS sets)
{
	int retval;

	//창크기 조절
	system("mode con cols=75 lines=25");
	setScreenBufferSize(75, 1000);

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
	serveraddr.sin_addr.s_addr = inet_addr(sets.server_ip);
	serveraddr.sin_port = htons(sets.server_mainPort);

	//서버에 접속하기
	printf("\n서버에 연결하는 중... \n");
	retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("connect()");
	

	//-----------------------------------------------------------------------------
	//서버에 접속할 경우,
	//클라이언트와 서버의 아이디, 닉네임을 교환한다.
	retval = client_exchangeIdNickname(sock, &sets);
	if (retval != 0)
	{
		closesocket(sock);
		return 1;
	}

	//접속한 서버의 정보 출력
	textcolor(SKY_BLUE);
	printf("서버에 연결했습니다. \n");
	printf("아이디: %d, 닉네임: %s \n\n", sets.server_uid, sets.server_nickName);
	textcolor(RESET);

	//재생목록을 만든다. 재생목록 배열은 100 * 512이다.
	//재생목록 배열에서 0번 행은 쓰지 않는다. 따라서 총 99개의 재생목록을 저장할 수 있다.
	//재생목록에서 안쓰는 부분은 반드시 Null값으로 초기화한다.
	char playlist[100][512] = { "\0" };

	//전체 재생목록을 수신한다.
	textcolor(YELLOW);
	printf("전체 재생목록 수신 중. \n");
	printf("잠시만 기다려 주십시오... \n");
	textcolor(RESET);
	double allRecvBytes = 0.0;
	retval = recvFullPlayList(sock, playlist, &allRecvBytes);
	if (retval != 0)
	{
		printf("전체 재생목록 수신 오류. \n");
		printf("프로그램을 종료합니다. \n");
		return 1;
	}
	else
		printf("전체 재생목록 수신 완료! (%0.2lfMB)\n", allRecvBytes / 1024 / 1024);

	//재생목록에 있는 파일을 서버와 동기화하면서 재생한다.
	while (1)
	{
		//콘솔 창 지움
		system("cls");
		setScreenBufferSize(75, 1000);

		//전체 재생목록을 출력한다.
		textcolor(WHITE);
		printf("\n");
		printFullPlaylist(playlist);
		printf("\n\n");
		textcolor(RESET);

		//재생목록이 비었으면 채워질 때까지 대기한다.
		if (strlen(playlist[1]) == 0)
		{
			textcolor(YELLOW);
			printf("재생목록이 비었습니다. \n");
			textcolor(RESET);
			printf("재생목록이 채워질 때까지 대기합니다. \n");
			Sleep(5000);

			//재생목록을 새로고침한다.
			retval = refreshPlaylist(sock, playlist);
			if (retval != 0)
				break;
			else
				continue;
		}

		//재생목록에서 1번을 재생한다.
		retval = client_MusicPlayer(playlist[1]);
		if (retval != 0)
			break;

		//재생을 완료한 파일을 삭제한다.
		retval = remove(playlist[1]);
		if (retval != 0)
		{
			printf("재생파일 삭제에 실패했습니다. \n");
			return 1;
		}

		//재생목록을 새로고침한다.
		retval = refreshPlaylist(sock, playlist);
		if (retval != 0)
			break;
	}
	//---------------------------------------------------------------------------------


	//서버와 연결을 종료한다.
	closesocket(sock);

	//프로그램을 종료한다.
	printf("\n프로그램을 종료합니다. \n");
	WSACleanup();
	return 0;
}
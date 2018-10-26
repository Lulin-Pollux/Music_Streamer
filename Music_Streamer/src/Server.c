#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment(lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"

#define STREAMING_PORT 50000

char playList[50][256];  //재생목록


//재생목록을 초기화하는 함수
int initializePlayList(int *fileCount)
{
	strcpy_s(playList[0], 256, "1.mp3");
	strcpy_s(playList[1], 256, "2.mp3");
	strcpy_s(playList[2], 256, "3.mp3");
	strcpy_s(playList[3], 256, "4.mp3");

	*fileCount = 4;
	return 0;
}

//재생목록을 동기화하는 함수
int synchronizePlayList(SOCKET client_sock, int fileCount)
{
	int retval;
	char buffer[100];
	char fileName[256];  //mp3 파일의 이름 저장
	char fileFolder[100];  //mp3 파일이 저장된 폴더 저장
	char filePath[1000];  //mp3 파일의 경로를 저장

	//파일이 저장된 폴더를 설정한다.
	strcpy_s(fileFolder, sizeof(fileFolder), ".\\playQue");

	//클라이언트가 받아야하는 mp3파일 개수를 전송한다.
	sprintf_s(buffer, sizeof(buffer), "%d", fileCount);
	send(client_sock, buffer, 2, 0);

	//재생목록에 해당하는 파일들을 모두 전송한다.
	for (int i = 0; i <= fileCount; i++)
	{
		//파일의 경로를 만든다.
		strcpy_s(filePath, sizeof(filePath), fileFolder);
		strcat_s(filePath, sizeof(filePath), "\\");
		strcat_s(filePath, sizeof(fileFolder), playList[i]);

		//파일을 송신한다.
		retval = fileSend(client_sock, fileName, filePath);
		if (retval != 0)
			return 1;
	}
	
	return 0;
}

//서버 메인 함수
int server()
{
	int retval;

	//재생목록을 초기화한다.
	int fileCount = 0;
	initializePlayList(&fileCount);

	//윈속 초기화
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	//대기용 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);

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

		//재생목록을 동기화한다.
		retval = synchronizePlayList(client_sock, fileCount);
		if (retval == 0)
		{
			printf("동기화 성공! \n");
			printf("주소= %s:%d \n\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		}
		else
			printf("동기화 실패! \n");

		//연결 종료
		closesocket(client_sock);
		return 0;
	}
	
	//----------------------------------------
	printf("서버를 종료합니다. \n");
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32")
#pragma comment(lib, "mswsock")
#include <stdio.h>
#include <WinSock2.h>
#include "ClassLinker.h"

#define SERVER_PORT 50000


int server()
{
	int retval;

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
	serveraddr.sin_port = htons(SERVER_PORT);
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

	//클라이언트의 접속을 기다림
	printf("클라이언트의 접속을 기다리는 중. . . \n\n");
	client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
	if (client_sock == INVALID_SOCKET)
		err_display("accept()");

	//클라이언트가 접속함
	printf("클라이언트가 접속하였습니다. \n");
	printf("주소= %s:%d \n\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	//파일의 경로와 이름을 가져온다.
	char filePath[1000];  //오디오 파일의 경로 저장
	char fileName[256];  //오디오 파일의 이름 저장
	retval = GetFilePath(filePath, sizeof(filePath), fileName);
	if (retval == 1)
	{
		printf("해당 파일명의 파일이 없습니다. \n");
		return 1;
	}

	//파일이 위치한 디렉토리로 이동한다.
	SetCurrentDirectory(filePath);

	//파일을 송신한다.
	if (fileSend(client_sock, fileName) == 0)
		printf("'%s' 송신 성공! \n\n", fileName);

	//연결 종료
	closesocket(client_sock);


	printf("서버를 종료합니다. \n");
	closesocket(listen_sock);
	WSACleanup();
	return 0;
}
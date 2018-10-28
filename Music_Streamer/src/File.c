#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include "ClassLinker.h"


//파일 송신 함수 (경로는 최대 512바이트)
int sendFile(SOCKET sock, _In_ char *filePath, _Out_ double *sendByte)
{
	/* 파일 송신 프로토콜
	1. 파일 경로를 전송한다.
	2. 파일 크기를 전송한다.
	3. 파일 데이터를 전송한다. */

	int retval;

	//1. 파일 경로를 전송한다. (512바이트 고정 길이)
	retval = send(sock, filePath, 512, 0);
	if (retval == SOCKET_ERROR)
		err_quit("파일 이름send()");

	//1-1. 보낼 파일을 연다.
	HANDLE hFile = CreateFileA(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "파일 열기 CreateFile() 오류.\n");
		system("pause");
		exit(1);
	}

	//2. 파일 크기를 전송한다. (4바이트 고정 길이)
	unsigned int fileSize = GetFileSize(hFile, NULL);
	retval = send(sock, (char*)&fileSize, sizeof(int), 0);
	if (retval == SOCKET_ERROR)
		err_quit("파일 크기send()");

	//3. 파일 데이터를 전송한다. (가변 길이)
	retval = TransmitFile(sock, hFile, 0, 0, NULL, NULL, 0);
	if (retval == FALSE)
	{
		fprintf(stderr, "파일 데이터 전송 TransmitFile() 오류.\n");
		system("pause");
		exit(1);
	}

	//전송 결과 내보내기
	*sendByte = fileSize;

	return 0;
}

//재생목록에 있는 모든 파일 전송 함수
int sendFullPlaylist(SOCKET sock, char playlist[][512])
{
	int retval;

	//재생목록을 전송한다. (51200바이트 고정 길이)
	retval = send(sock, playlist[0], 51200, 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("재생목록 send()");
		return 1;
	}

	//파일 전송
	double sendByte;
	for (int i = 1; i < 100; i++)
	{
		if (strlen(playlist[i]) == 0)
			break;

		sendFile(sock, playlist[i], &sendByte);
		printf("전송 완료! (%0.2lfMB) \n", sendByte / 1024 / 1024);
	}

	return 0;
}

//파일 수신 함수 (경로는 최대 512바이트)
int recvFile(SOCKET sock, _Out_ char *filePath, _Out_ double *recvByte)
{
	/* 파일 수신 프로토콜
	1. 파일 경로를 수신한다.
	2. 파일 크기를 수신한다.
	3. 파일 데이터를 수신한다. */

	int retval;

	//1. 파일 경로를 수신한다. (512바이트 고정 길이)
	retval = recv(sock, filePath, 512, MSG_WAITALL);
	if (retval == SOCKET_ERROR)
		err_quit("파일 이름recv()");

	//1-1. 수신할 파일을 쓰기모드로 연다.
	FILE *wfp;
	retval = fopen_s(&wfp, filePath, "wb");
	if (retval != 0)
	{
		perror("파일 열기fopen_s()");
		system("pause");
		exit(1);
	}

	//2. 파일 크기를 수신한다. (4바이트 고정 길이)
	unsigned int fileSize = 0;
	retval = recv(sock, (char*)&fileSize, sizeof(int), MSG_WAITALL);
	if (retval == SOCKET_ERROR)
		err_quit("파일 크기recv()");

	//3. 파일 데이터를 수신한다.
	char *buffer = (char*)calloc(fileSize, sizeof(char));
	retval = recv(sock, buffer, fileSize, MSG_WAITALL);
	if (retval == SOCKET_ERROR)
		err_quit("파일 데이터recv()");

	//3-1. 수신한 데이터를 파일에 쓴다.
	fwrite(buffer, 1, retval, wfp);
	if (ferror(wfp))
	{
		perror("파일 쓰기fwrite()");
		system("pause");
		exit(1);
	}

	//파일 포인터, 동적 메모리 반환
	fclose(wfp);
	free(buffer);

	//전송 결과 내보내기
	*recvByte = retval;

	return 0;
}

//재생목록에 있는 모든 파일 수신 함수
int recvFullPlayList(SOCKET sock, char playList[][512])
{
	int retval;

	//재생목록을 수신한다. (51200바이트 고정 길이)
	retval = recv(sock, playList[0], 51200, MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("재생목록 recv()");
		return 1;
	}

	//파일 수신
	double recvByte;
	for (int i = 1; i < 100; i++)
	{
		if (strlen(playList[i]) == 0)
			break;

		recvFile(sock, playList[i], &recvByte);
		printf("수신 완료! (%0.2lfMB)\n", recvByte / 1024 / 1024);
	}

	return 0;
}

//설정파일을 불러오는 함수
int importSettings(SETTINGS *sets)
{
	int retval;
	char buffer[256];

	//설정파일을 읽기모드로 연다.
	FILE *rfp;
	retval = fopen_s(&rfp, "Settings.ini", "r");
	if (retval != 0)
	{
		perror("설정파일 열기fopen_s()");
		return 1;
	}

	//설정파일을 읽어온다.
	while (!feof(rfp))
	{
		//한 줄씩 파일을 읽어온다.
		fgets(buffer, sizeof(buffer), rfp);

		//예외사항을 적어둔다.
		if (strncmp(buffer, "//", 2) == 0)  //주석문 읽기안함
			continue;
		else if (strcmp(buffer, "\n") == 0)  //Enter 읽기안함
			continue;
		else if (strncmp(buffer, " ", 1) == 0)  //맨 앞이 공백인 문장 읽기안함
			continue;

		else
		{
			//'\n'문자 제거
			int len = (int)strlen(buffer);
			if (buffer[len - 1] == '\n')
				buffer[len - 1] = '\0';

			//'='문자 위치를 저장한다.
			//만약, 값이 없으면 읽지않는다.
			char *pos = strchr(buffer, '=');
			if (strcmp(pos + 1, "\0") == 0)
				continue;

			//설정 항목들을 sets객체에 각각 저장한다.
			//설정 항목은 if문에 적어둔다.
			//-------------------------------------------------------------------------
			if (strncmp(buffer, "execute_mode", pos - buffer) == 0)
				strcpy_s(sets->execute_mode, sizeof(sets->execute_mode), pos + 1);
			else if (strncmp(buffer, "server_ip", pos - buffer) == 0)
				strcpy_s(sets->server_ip, sizeof(sets->server_ip), pos + 1);
			else if (strncmp(buffer, "server_mainPort", pos - buffer) == 0)
				sets->server_mainPort = atoi(pos + 1);
			else if (strncmp(buffer, "server_requestPort", pos - buffer) == 0)
				sets->server_requestPort = atoi(pos + 1);
			else if (strncmp(buffer, "server_uid", pos - buffer) == 0)
				sets->server_uid = atoi(pos + 1);
			else if (strncmp(buffer, "server_nickName", pos - buffer) == 0)
				strcpy_s(sets->server_nickName, sizeof(sets->server_nickName), pos + 1);
			else if (strncmp(buffer, "client_uid", pos - buffer) == 0)
				sets->client_uid = atoi(pos + 1);
			else if (strncmp(buffer, "client_nickName", pos - buffer) == 0)
				strcpy_s(sets->client_nickName, sizeof(sets->client_nickName), pos + 1);
			//-------------------------------------------------------------------------
		}
	}

	fclose(rfp);
	return 0;
}

//재생목록을 초기화하는 함수
int initializePlaylist(char playlist[][512])
{
	int retval;
	char buffer[512];

	//초기 재생목록 파일을 읽기모드로 연다.
	FILE *rfp;
	retval = fopen_s(&rfp, "./playQue/초기 재생목록.txt", "r");
	if (retval != 0)
	{
		perror("초기 재생목록 파일fopen_s()");
		return 1;
	}

	//초기 재생목록 파일을 읽어온다.
	int i = 1;
	while (!feof(rfp))
	{
		fgets(buffer, 512, rfp);
		if (strncmp(buffer, "//", 2) == 0)  //주석문 읽기안함
			continue;
		else if (strcmp(buffer, "\n") == 0)  //Enter 읽기안함
			continue;
		else
		{
			//'\n'문자 제거
			int len = (int)strlen(buffer);
			if (buffer[len - 1] == '\n')
				buffer[len - 1] = '\0';

			strcpy_s(playlist[i], 512, "./playQue/");
			strcat_s(playlist[i], 512, buffer);
		}
		i++;
	}

	fclose(rfp);
	return 0;
}
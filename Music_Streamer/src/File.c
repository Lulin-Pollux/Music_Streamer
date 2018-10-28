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
int importSettings(SETTINGS *setUp)
{
	int retval;

	FILE *ini_rfp;
	retval = fopen_s(&ini_rfp, "Settings.ini", "r");
	if (retval != 0)
		return 1;
	else
	{
		char buffer[10][100];  //버퍼
		char value[30];  //설정값

		int i = 0;
		while (!feof(ini_rfp))
		{
			//한 줄씩 파일을 읽어온다.
			fgets(buffer[i], 100, ini_rfp);
			if ((strncmp(buffer[i], "//", 2) == 0) || (strchr(buffer[i], '=') == NULL))
				continue;

			//'\n'문자 제거
			int len = (int)strlen(buffer[i]);
			if (buffer[i][len - 1] == '\n')
				buffer[i][len - 1] = '\0';
			i++;
		}

		for (int k = 0; k < i; k++)
		{
			//버퍼에서 값 가져오기
			char *pos = strchr(buffer[k], '=');
			strcpy_s(value, sizeof(value), pos + 1);

			//설정 항목들을 setUp객체에 각각 저장한다.
			if (strncmp(buffer[k], "execute_mode", pos - buffer[k]) == 0)
				strcpy_s(setUp->execute_mode, sizeof(setUp->execute_mode), value);
			else if (strncmp(buffer[k], "server_ip", pos - buffer[k]) == 0)
				strcpy_s(setUp->server_ip, sizeof(setUp->server_ip), value);
			else if (strncmp(buffer[k], "uid", pos - buffer[k]) == 0)
				setUp->uid = atoi(value + 1);
			else if (strncmp(buffer[k], "nickName", pos - buffer[k]) == 0)
				strcpy_s(setUp->nickName, sizeof(setUp->nickName), value);
		}
	}

	return 0;
}

//파일의 절대경로와 이름을 넘겨주는 함수
int getFilePathName(char *filePath, int filePath_len, char *fileName)
{
	int retval;
	char buffer[256];

	//파일명 입력받기
	printf("파일명 또는 경로를 입력하세요. \n");
	printf(">>> ");
	gets_s(buffer, sizeof(buffer));

	//파일이 존재하는지 검사한다.
	FILE *rfp;
	retval = fopen_s(&rfp, buffer, "r");
	if (retval != 0)
		return 1;
	else
		fclose(rfp);

	//파일의 절대경로를 넘겨준다.
	GetFullPathName(buffer, filePath_len, filePath, NULL);

	//파일의 이름을 넘겨준다.
	strcpy_s(fileName, 256, strrchr(filePath, '\\') + 1);
	return 0;
}
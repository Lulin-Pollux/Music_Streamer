#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include "ClassLinker.h"


//파일을 송신하는 함수
int fileSend(SOCKET sock, _In_ char *fileName)
{
	int retval;

	//파일 이름 보내기(256바이트 고정 길이)
	retval = send(sock, fileName, sizeof(fileName), 0);
	if (retval == SOCKET_ERROR)
	{
		err_display("send()");
		return 1;
	}

	//전송할 파일을 연다.
	HANDLE hFile = CreateFileA(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		printf("CreateFile() 오류! \n");
		return 1;
	}

	//TransmitFile() 함수로 데이터를 전송한다.
	retval = TransmitFile(sock, hFile, 0, 0, NULL, NULL, 0);
	if (retval != TRUE) {
		printf("TransmitFile() 오류! \n");
		return 1;
	}

	//전송했던 파일을 닫는다.
	DeleteFileA(fileName);
	CloseHandle(hFile);
	return 0;
}

//파일을 수신하는 함수
int fileReceive(SOCKET sock, _Out_ char *received_fileName, _Out_ float *received_fileSize)
{
	char *buffer = (char*)malloc(sizeof(char) * (8192 * 1024));
	int retval;

	//파일 이름 받기(256바이트 고정 길이)
	char fileName[256];
	retval = recv(sock, fileName, 256, MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("fileName_recv()");
		return 2;
	}

	//쓰기용 파일 열기 (없으면 생성, 있으면 덮어씌움)
	FILE *wfp;
	fopen_s(&wfp, fileName, "wb");
	if (wfp == NULL) {
		perror("fopen_s()");
		return 2;
	}

	//파일 데이터 받기
	int totalBytes = 0;
	while (1) {
		retval = recv(sock, buffer, sizeof(buffer), 0);
		if (retval == SOCKET_ERROR) {
			err_display("fileData_recv()");
			break;
		}
		else if (retval == 0)
			break;
		else {
			fwrite(buffer, 1, retval, wfp);
			if (ferror(wfp)) {
				perror("fwrite()");
				break;
			}
			totalBytes += retval;
		}
	}
	fclose(wfp);

	//전송 결과 출력
	if (retval == 0)
	{
		strcpy_s(received_fileName, 256, fileName);
		*received_fileSize = totalBytes / 1024.0f / 1024.0f;
		return 0;
	}
	else
	{
		printf("'%s' 수신 실패! \n\n", fileName);
		return 1;
	}
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
			strcpy_s(value, sizeof(value), pos);

			if (strncmp(buffer[k], "execute_mode", pos - buffer[k]) == 0)
				strcpy_s(setUp->execute_mode, sizeof(setUp->execute_mode), value + 1);
			else if (strncmp(buffer[k], "server_ip", pos - buffer[k]) == 0)
				strcpy_s(setUp->server_ip, sizeof(setUp->server_ip), value + 1);
			else if (strncmp(buffer[k], "uid", pos - buffer[k]) == 0)
				setUp->uid = atoi(value + 1);
			else if (strncmp(buffer[k], "nickName", pos - buffer[k]) == 0)
				strcpy_s(setUp->nickName, sizeof(setUp->nickName), value + 1);
		}
	}

	return 0;
}
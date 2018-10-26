#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <MSWSock.h>
#include "ClassLinker.h"


//파일을 송신하는 함수
int fileSend(SOCKET sock, _In_ char *fileName, _In_ char *filePath)
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
	HANDLE hFile = CreateFileA(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
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
int fileReceive(SOCKET sock, _In_ char *save_directory, _Out_ char *saved_fileName, _Out_ float *saved_fileSize)
{
	char *buffer = (char*)malloc(sizeof(char) * (8192 * 1024));
	int retval;

	//파일 이름 받기(256바이트 고정 길이)
	char fileName[256];
	retval = recv(sock, fileName, 256, MSG_WAITALL);
	if (retval == SOCKET_ERROR)
	{
		err_display("fileName_recv()");
		return 2;
	}

	char save[500];
	strcpy_s(save, sizeof(save), save_directory);
	strcat_s(save, sizeof(save), "\\");
	strcat_s(save, sizeof(save), fileName);

	//쓰기용 파일 열기 (없으면 생성, 있으면 덮어씌움)
	FILE *wfp;
	fopen_s(&wfp, save, "wb");
	if (wfp == NULL)
	{
		perror("fopen_s()");
		return 2;
	}

	//파일 데이터 받기
	int totalBytes = 0;
	while (1) {
		retval = recv(sock, buffer, sizeof(buffer), 0);
		if (retval == SOCKET_ERROR)
		{
			err_display("fileData_recv()");
			break;
		}
		else if (retval == 0)
			break;
		else {
			fwrite(buffer, 1, retval, wfp);
			if (ferror(wfp))
			{
				perror("fwrite()");
				break;
			}
			totalBytes += retval;
		}
	}
	free(buffer);
	fclose(wfp);

	//전송 결과 출력
	if (retval == 0)
	{
		strcpy_s(saved_fileName, 256, fileName);
		*saved_fileSize = totalBytes / 1024.0f / 1024.0f;
		return 0;
	}
	else
	{
		//수신 실패!
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
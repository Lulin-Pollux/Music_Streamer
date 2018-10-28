#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#include <Windows.h>
#include "ClassLinker.h"


int player(char *filePath)
{
	int retval;
	MCI_OPEN_PARMS mciOpen;  //장치 드라이버를 열 때 보고서
	MCI_PLAY_PARMS mciPlay;  //재생할 때 보고서
	DWORD deviceID;  //장치 드라이버 ID 저장
	char fileName[256];  //오디오 파일의 이름 저장

	//파일의 이름을 경로에서 분리 저장한다.
	strcpy_s(fileName, sizeof(fileName), strrchr(filePath, '\\') + 1);

	//열 파일의 이름과 형태를 기술한다.
	mciOpen.lpstrElementName = filePath;
	mciOpen.lpstrDeviceType = "mpegvideo";

	//사운드 장치 드라이버 열기
	retval = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD_PTR)&mciOpen);
	if (retval != 0)
	{
		char buffer[100];
		mciGetErrorString(retval, buffer, sizeof(buffer));
		printf("%s \n", buffer);
		return 2;
	}

	//파일을 재생한다.
	deviceID = mciOpen.wDeviceID;	//다바이스 ID를 받는다.
	retval = mciSendCommand(deviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mciPlay);
	if (retval != 0)
	{
		char buffer[100];
		mciGetErrorString(retval, buffer, sizeof(buffer));
		printf("%s \n", buffer);
	}
	else
		printf("'%s'를 재생합니다. \n", fileName);

	//재생하는 동안 기다림
	system("pause");

	//사운드 장치 드라이버 닫기
	mciSendCommand(deviceID, MCI_CLOSE, MCI_NOTIFY, (DWORD_PTR)NULL);
	return 0;
}
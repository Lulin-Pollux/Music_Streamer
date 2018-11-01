#include <stdio.h>
#include <Windows.h>
#include <Digitalv.h>
#include "ClassLinker.h"

#define SEEK_TO_START 0
#define SEEK_TO_END 1
#define SEEK_TO 2


//MCI 오류를 출력하는 함수
int printMciError(int errorCode)
{
	int retval;

	char buffer[128];
	retval = mciGetErrorString(errorCode, buffer, sizeof(buffer));
	if (retval != TRUE)
	{
		printf("해당 에러코드를 알 수 없습니다. \n");
		return 1;
	}
	else
		printf("%s \n", buffer);

	return 0;
}

//오디오 파일을 닫는 함수
int closeAudioFile(MCIDEVICEID deviceID)
{
	int retval;

	//사운드 장치 드라이버 닫기
	retval = mciSendCommand(deviceID, MCI_CLOSE, MCI_NOTIFY, 0);
	if (retval != 0)
		printMciError(retval);

	return 0;
}

//오디오 파일을 재생하는 함수
int playAudioFile(MCIDEVICEID deviceID, MCI_PLAY_PARMS *mciPlay)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mciPlay);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//오디오 파일 재생을 일시정지하는 함수
//메모리에 오디오 파일을 유지시켜, 다시재생할 때 빠르게 재생한다.
int pauseAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS *mciGeneric)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_PAUSE, MCI_NOTIFY, (DWORD_PTR)&mciGeneric);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//오디오 파일 재생을 재개하는 함수
int resumeAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS *mciGeneric)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_RESUME, MCI_NOTIFY, (DWORD_PTR)&mciGeneric);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//오디오 파일 재생을 정지하는 함수(mciGeneric = NULL)
//일시정지와 다르게 메모리에서 오디오 파일을 내려버린다.
int stopAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS *mciGeneric)
{
	int retval;
	
	retval = mciSendCommand(deviceID, MCI_STOP, MCI_NOTIFY, (DWORD_PTR)&mciGeneric);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//재생 위치를 이동시키는 함수
int seekAudioFile(MCIDEVICEID deviceID, MCI_SEEK_PARMS *mciSeek, int seekControl)
{
	int retval;

	switch (seekControl)
	{
	//시작 위치로 이동
	case SEEK_TO_START:
		retval = mciSendCommand(deviceID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD_PTR)&mciSeek);
		if (retval != 0)
		{
			printMciError(retval);
			return 1;
		}
		break;
	//끝 위치로 이동
	case SEEK_TO_END:
		retval = mciSendCommand(deviceID, MCI_SEEK, MCI_SEEK_TO_END, (DWORD_PTR)&mciSeek);
		if (retval != 0)
		{
			printMciError(retval);
			return 1;
		}
		break;
	//특정 위치로 이동
	case SEEK_TO:
		retval = mciSendCommand(deviceID, MCI_SEEK, MCI_TO, (DWORD_PTR)&mciSeek);
		if (retval != 0)
		{
			printMciError(retval);
			return 1;
		}
		break;
	}

	return 0;
}
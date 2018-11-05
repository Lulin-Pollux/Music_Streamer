#include <stdio.h>
#include <Windows.h>
#include "ClassLinker.h"


//MCI 오류를 출력하는 함수
int printMciError(int errorCode)
{
	int retval;

	char buffer[128];
	MessageBeep(MB_ICONERROR);
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

//각 모드에 알맞는 설명을 출력하는 함수
int printModeDescription(int mode)
{
	textcolor(YELLOW);
	switch (mode)
	{
	case MCI_MODE_NOT_READY:
		printf("장치 드라이버가 준비되지 않았습니다. \n");
		break;
	case MCI_MODE_OPEN:
		printf("오디오 파일을 열었습니다. \n");
		break;
	case MCI_MODE_PLAY:
		printf("재생 중 \n");
		break;
	case MCI_MODE_PAUSE:
		printf("일시 정지 \n");
		break;
	case MCI_MODE_RECORD:
		printf("녹음 중 \n");
		break;
	case MCI_MODE_SEEK:
		printf("재생 위치 옮김 \n");
		break;
	case MCI_MODE_STOP:
		printf("정지 \n");
		break;
	default:
		printf("알 수 없는 모드입니다. \n");
		return 1;
	}
	textcolor(RESET);

	return 0;
}

//오디오 파일을 여는 함수
int openAudioFile(_Out_ MCIDEVICEID *deviceID, MCI_OPEN_PARMS mciOpenParms)
{
	int retval;

	retval = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD_PTR)&mciOpenParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}
	
	//DeviceID를 변수에 저장한다.
	*deviceID = mciOpenParms.wDeviceID;

	return 0;
}

//오디오 파일을 닫는 함수
int closeAudioFile(MCIDEVICEID deviceID)
{
	int retval;

	//사운드 장치 드라이버 닫기
	retval = mciSendCommand(deviceID, MCI_CLOSE, MCI_WAIT, 0);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//오디오 파일을 재생하는 함수
int playAudioFile(MCIDEVICEID deviceID, MCI_PLAY_PARMS mciPlayParms)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mciPlayParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//오디오 파일 재생을 일시정지하는 함수
//메모리에 오디오 파일을 유지시켜, 다시 재생할 때 빠르게 재생한다.
int pauseAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS mciGenericParms)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_PAUSE, MCI_NOTIFY, (DWORD_PTR)&mciGenericParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//오디오 파일 재생을 재개하는 함수
int resumeAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS mciGenericParms)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_RESUME, MCI_NOTIFY, (DWORD_PTR)&mciGenericParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//오디오 파일 재생을 일시정지하는 함수
//메모리에서 오디오 파일을 내려버린다.
int stopAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS mciGenericParms)
{
	int retval;
	
	retval = mciSendCommand(deviceID, MCI_STOP, MCI_NOTIFY, (DWORD_PTR)&mciGenericParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//시간 형식을 설정하는 함수
int setTimeFormat(MCIDEVICEID deviceID, MCI_SET_PARMS mciSetParms, DWORD timeFormat)
{
	int retval;

	mciSetParms.dwTimeFormat = timeFormat;
	retval = mciSendCommand(deviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSetParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//재생 위치를 이동시키는 함수 (단위: Miliseconds)
int seekAudioFile(MCIDEVICEID deviceID, MCI_SEEK_PARMS mciSeekParms, DWORD seekTo_ms)
{
	int retval;

	mciSeekParms.dwTo = seekTo_ms;
	retval = mciSendCommand(deviceID, MCI_SEEK, MCI_TO, (DWORD_PTR)&mciSeekParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//현재 모드를 가져오는 함수
int getCurrentMode(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms, _Out_ int *mode)
{
	int retval;

	//명령 전달
	mciStatusParms.dwItem = MCI_STATUS_MODE;
	retval = mciSendCommand(deviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	//현재 모드의 정수값을 내보낸다.
	if (mode != NULL)
		*mode = (int)mciStatusParms.dwReturn;

	return 0;
}

//재생할 파일의 총 길이를 가져오는 함수
int getAudioFileLength(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms, _Out_ unsigned int *totalLen_ms)
{
	int retval;

	mciStatusParms.dwItem = MCI_STATUS_LENGTH;
	retval = mciSendCommand(deviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}
	*totalLen_ms = (unsigned int)mciStatusParms.dwReturn;

	return 0;
}

//현재 재생위치를 가져오는 함수
int getCurrentPosition(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms, _Out_ unsigned int *position_ms)
{
	int retval;
	
	mciStatusParms.dwItem = MCI_STATUS_POSITION;
	retval = mciSendCommand(deviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}
	*position_ms = (unsigned int)mciStatusParms.dwReturn;

	return 0;
}
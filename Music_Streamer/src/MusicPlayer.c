#pragma comment(lib, "winmm.lib")
#include <stdio.h>
#include <Windows.h>
#include "ClassLinker.h"


//전체 재생목록을 출력한다.
int printFullPlaylist(char playlist[][512])
{
	printf("\n");
	printf("\t 재생목록 \n");
	printf("--------------------------------\n");
	for (int i = 1; i < 100; i++)
	{
		if (strlen(playlist[i]) == 0)
			break;
		else
		{
			char *pos = strrchr(playlist[i], '/');
			printf("%d. %s\n", i, pos + 1);
		}
	}
	printf("\n");

	return 0;
}

//재생목록을 추가하는 함수
int addPlaylist(char *fileName, char playlist[][512])
{
	int retval;
	char buffer[512];

	//앞에 상대경로를 붙이고 뒤에 파일명을 붙인다.
	strcpy_s(buffer, 512, "./playQue/");
	strcat_s(buffer, 512, fileName);

	//파일이름에 해당하는 파일이 존재하는지 검사한다.
	FILE *rfp;
	retval = fopen_s(&rfp, buffer, "r");
	if (retval != 0)
	{
		textcolor(YELLOW);
		printf("해당 파일이 없습니다. \n");
		textcolor(RESET);
		return 1;
	}
	else
		fclose(rfp);

	//재생목록의 끝을 알아낸다.
	int eol = -1;
	for (int i = 1; i < 100; i++)
	{
		if (strlen(playlist[i]) == 0)
		{
			eol = i;
			break;
		}
	}

	//재생목록이 꽉찬경우, 종료한다.
	if (eol < 0)
	{
		textcolor(YELLOW);
		printf("재생목록에 빈 공간이 없습니다. \n");
		textcolor(RESET);
		return 1;
	}
	else
	{
		//재생목록에 추가한다.
		strcpy_s(playlist[eol], 512, buffer);
	}

	return 0;
}

//재생목록을 삭제하는 함수
int deletePlaylist(int row, char playlist[][512])
{
	//지우는 곳을 기준으로 맨 끝까지 각각 1칸씩 올린다.
	for (int i = row; i <= 98; i++)
		strcpy_s(playlist[i], 512, playlist[i + 1]);

	//맨 끝행을 지운다.
	for (int i = 0; i < 512; i++)
		playlist[99][i] = '\0';

	return 0;
}

//서버용 음악 플레이어


//클라이언트용 음악 플레이어
int client_MusicPlayer(char *filePath)
{
	int retval;

	MCI_OPEN_PARMS mciOpen;		//장치 드라이버를 열 때 필요한 객체
	MCI_PLAY_PARMS mciPlay;		//재생할 때 필요한 객체
	MCIDEVICEID deviceID;	//장치 드라이버 ID 저장
	char fileName[256];		//오디오 파일의 이름 저장
	char fileType[10];		//파일의 형식을 저장

	//파일의 경로와 이름, 확장자를 분리해서 저장한다.
	strcpy_s(fileName, sizeof(fileName), strrchr(filePath, '/') + 1);
	strcpy_s(fileType, sizeof(fileType), strrchr(filePath, '.'));

	//열 파일의 이름과 형태를 기술한다.
	mciOpen.lpstrElementName = filePath;
	if (strcmp(fileType, ".mp3") == 0)
		mciOpen.lpstrDeviceType = "mpegvideo";
	else if (strcmp(fileType, ".wav") == 0)
		mciOpen.lpstrDeviceType = "waveaudio";
	else if (strcmp(fileType, ".mid") == 0)
		mciOpen.lpstrDeviceType = "sequencer";
	else
	{
		printf("해당 파일형식은 지원하지 않습니다. \n");
		return 1;
	}

	//사운드 장치 드라이버 열기
	retval = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD_PTR)&mciOpen);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}
	else
		deviceID = mciOpen.wDeviceID;  //DeviceID를 변수에 저장한다.

	//파일을 재생한다.
	textcolor(SKY_BLUE);
	printf("'%s'를 재생합니다. \n", fileName);
	textcolor(RESET);
	retval = mciSendCommand(deviceID, MCI_PLAY, MCI_WAIT, (DWORD_PTR)&mciPlay);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	//오디오 파일을 닫는다.
	closeAudioFile(deviceID);

	return 0;
}
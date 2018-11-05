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
int insertPlaylist(char *fileName, char playlist[][512])
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

//재생상태를 보여주는 함수
int showPlayStatus(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms)
{
	//현재 모드를 보여준다.
	int mode;
	getCurrentMode(deviceID, mciStatusParms, &mode);

	//재생할 파일의 총 길이와 현재 위치를 가져온다.
	unsigned int totalLen_ms = 0;
	unsigned int position_ms = 0;
	getAudioFileLength(deviceID, mciStatusParms, &totalLen_ms);
	getCurrentPosition(deviceID, mciStatusParms, &position_ms);

	//재생 진행 막대를 만든다.
	//재생 중이 아니거나 총 길이와 현재 위치가 같으면 리턴한다.
	do
	{
		//커서를 행의 맨 앞으로 옮김
		printf("\r");

		//현재 위치를 가져온다.
		getCurrentPosition(deviceID, mciStatusParms, &position_ms);

		//현재 위치를 표시한다.
		printf(" %d:%02d", position_ms / 1000 / 60, position_ms / 1000 % 60);
		printf("  ");

		//진행 바를 만든다.
		if (position_ms == 0)
			printf("--------------------------------------------------");
		else
		{
			float percent = position_ms / (float)totalLen_ms * 100;
			int progress = (int)percent / 2;

			for (int i = 1; i <= progress; i++)
				printf("#");
			for (int i = 1; i <= 50 - progress; i++)
				printf("-");
		}

		//총 길이를 표시한다.
		printf("  ");
		printf("%d:%02d", totalLen_ms / 1000 / 60, totalLen_ms / 1000 % 60);
		Sleep(500);
	} while ((mode == MCI_MODE_PLAY) && (position_ms != totalLen_ms));

	return 0;
}

//클라이언트용 음악 플레이어
int client_MusicPlayer(char *filePath)
{
	int retval;

	MCIDEVICEID deviceID;	//장치 드라이버 ID 저장
	MCI_GENERIC_PARMS mciGenericParms = { 0 };  //일반 구조체 변수
	MCI_OPEN_PARMS mciOpenParms = { 0 };		//오디오 파일을 열 때 필요한 구조체 변수
	MCI_PLAY_PARMS mciPlayParms = { 0 };		//재생할 때 필요한 구조체 변수
	MCI_SEEK_PARMS mciSeekParms = { 0 };		//위치 옮길 때 필요한 구조체 변수
	MCI_SET_PARMS mciSetParms = { 0 };			//MCI 설정에 필요한 구조체 변수
	MCI_STATUS_PARMS mciStatusParms = { 0 };	//현재 상태를 받아올 때 필요한 구조체 변수
	char fileName[256];		//오디오 파일의 이름 저장
	char fileType[10];		//파일의 형식을 저장

	//파일의 경로와 이름, 확장자를 분리해서 저장한다.
	strcpy_s(fileName, sizeof(fileName), strrchr(filePath, '/') + 1);
	strcpy_s(fileType, sizeof(fileType), strrchr(filePath, '.'));

	//열 파일의 이름과 형태를 기술한다.
	mciOpenParms.lpstrElementName = filePath;
	if (strcmp(fileType, ".mp3") == 0)
		mciOpenParms.lpstrDeviceType = "mpegvideo";
	else if (strcmp(fileType, ".wav") == 0)
		mciOpenParms.lpstrDeviceType = "waveaudio";
	else if (strcmp(fileType, ".mid") == 0)
		mciOpenParms.lpstrDeviceType = "sequencer";
	else
	{
		printf("해당 파일형식은 지원하지 않습니다. \n");
		return 1;
	}

	//오디오 파일 열기
	retval = openAudioFile(&deviceID, mciOpenParms);
	if (retval != 0)
		return 1;

	//시간 형식을 밀리초로 설정한다.
	setTimeFormat(deviceID, mciSetParms, MCI_FORMAT_MILLISECONDS);

	//파일을 재생한다.
	retval = playAudioFile(deviceID, mciPlayParms);
	if (retval != 0)
		return 1;

	//재생하는 파일의 이름을 출력한다.
	textcolor(SKY_BLUE);
	printf("'%s'를 재생합니다. \n\n", fileName);
	textcolor(RESET);

	//현재 재생상태를 표시한다.
	showPlayStatus(deviceID, mciStatusParms);

	//오디오 파일을 닫는다.
	closeAudioFile(deviceID);

	return 0;
}
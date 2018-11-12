#include <stdio.h>
#include <Windows.h>
#include "ClassLinker.h"
#include "resource.h"


int main()
{
	int retval;

	//콘솔창 초기화
	SetConsoleTitle("Music Streamer");
	system("mode con cols=80 lines=35");
	setScreenBufferSize(80, 9000);

	//프로그램의 설정을 불러온다.
	SETTINGS sets;  //프로그램의 설정을 저장할 객체
	ZeroMemory(&sets, sizeof(sets));
	retval = importSettings(&sets);
	if (retval != 0)
	{
		textcolor(YELLOW);
		printf("설정파일이 누락되어 있습니다. \n");
		printf("설정파일을 복구해주세요. \n");
		textcolor(RESET);
		system("pause");
		return 1;
	}

	//멀티채팅 프로그램을 실행한다.
	FILE *rfp;
	retval = fopen_s(&rfp, "MultiChat.exe", "r");
	if (retval != 0)
	{
		textcolor(YELLOW);
		printf("멀티채팅 프로그램이 없습니다. \n");
		textcolor(RESET);
		system("pause");
		return 1;
	}
	else
	{
		fclose(rfp);
		system("start cmd /c MultiChat.exe");
	}

	//설정에서 실행모드가 서버면 서버실행, 아니면 클라이언트 실행
	//만약, morse나 sstv면 이스터에그 실행
	if (strcmp(sets.execute_mode, "server") == 0)
	{
		textcolor(YELLOW);
		printf("서버 모드로 실행합니다. \n");
		textcolor(RESET);
		server(sets);
	}
	else if (strcmp(sets.execute_mode, "client") == 0)
	{
		textcolor(YELLOW);
		printf("클라이언트 모드로 실행합니다. \n");
		textcolor(RESET);
		client(sets);
	}
	else if (strcmp(sets.execute_mode, "morse") == 0)
	{
		textcolor(GREEN);
		printf("\n");
		printf("interior transmission active external data line active message digest active.");
		printf("system data dump active user backup active password backup active.");
		printf("\n");
		textcolor(RESET);
		sndPlaySound(MAKEINTRESOURCE(IDR_WAVE1), SND_RESOURCE | SND_SYNC);
		Sleep(1000);
		return 0;
	}
	else if (strcmp(sets.execute_mode, "sstv") == 0)
	{
		textcolor(SKY_BLUE);
		printf("\n");
		printf("FB00BF4049C0A6C46D101FF11AF2C73A957D139D761F437877160DF535D09C5");
		printf("AE99780491B0DE660A09E3CBB87FA4D581F9BD0BAA96C153571C05B52AE4DC906");
		printf("\n");
		textcolor(RESET);
		sndPlaySound(MAKEINTRESOURCE(IDR_WAVE2), SND_RESOURCE | SND_SYNC);
		Sleep(1000);
		return 0;
	}
	else
	{
		printf("알 수 없는 실행모드입니다. \n");
		printf("실행모드를 정확히 적어주세요. \n");
		exit(1);
	}

	//프로그램 종료
	system("pause");
	return 0;
}
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include "ClassLinker.h"
#include "resource.h"


int main()
{
	int retval;

	//콘솔창 초기화
	SetConsoleTitle("Music Radio");
	system("mode con cols=90 lines=35");
	setScreenBufferSize(90, 9000);

	//프로그램의 설정을 불러온다.
	SETTINGS sets;  //프로그램의 설정을 저장할 객체
	ZeroMemory(&sets, sizeof(sets));
	retval = importSettings(&sets);
	if (retval != 0)
	{
		printf("설정파일이 누락되어 있습니다. \n");
		printf("설정파일을 복구해주세요. \n");
		system("pause");
		exit(1);
	}

	//설정에서 실행모드가 서버면 서버실행, 아니면 클라이언트 실행
	//만약, morse나 sstv면 이스터에그 실행
	if (strcmp(sets.execute_mode, "server") == 0)
	{
		printf("서버 모드로 실행합니다. \n");
		server(sets);
	}
	else if (strcmp(sets.execute_mode, "client") == 0)
	{
		printf("클라이언트 모드로 실행합니다. \n");
		client(sets);
	}
	else if (strcmp(sets.execute_mode, "morse") == 0)
	{
		sndPlaySound(MAKEINTRESOURCE(IDR_WAVE1), SND_RESOURCE | SND_ASYNC);
		printf("Morse 이스터 에그를 실행합니다. \n");
		printf("Morse 디코더로 분석해보세요~~ \n\n");
	}
	else if (strcmp(sets.execute_mode, "sstv") == 0)
	{
		sndPlaySound(MAKEINTRESOURCE(IDR_WAVE2), SND_RESOURCE | SND_ASYNC);
		printf("SSTV 신호입니다. \n");
		printf("SSTV 디코더로 출력해보세요~~ \n\n");
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
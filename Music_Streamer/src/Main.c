#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include "ClassLinker.h"


int main()
{
	int retval;
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
	if (strcmp(sets.execute_mode, "server") == 0)
		server(sets);
	else
		client(sets);

	//프로그램 종료
	system("pause");
	return 0;
}
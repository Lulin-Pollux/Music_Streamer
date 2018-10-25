#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include "ClassLinker.h"

SETTINGS setUp;  //프로그램의 설정을 저장할 객체


int main()
{
	SetConsoleTitle("Music Radio");
	system("mode con cols=90 lines=35");
	setScreenBufferSize(90, 9000);

	int retval;
	
	retval = importSettings(&setUp);
	if (retval == 1)
	{
		printf("설정파일이 누락되어 있습니다. \n");
		printf("설정파일을 복구해주세요. \n");
		system("pause");
		exit(1);
	}

	if (strcmp(setUp.execute_mode, "server") == 0)
		server();
	else if (strcmp(setUp.execute_mode, "client") == 0)
		client(&setUp);

	system("pause");
	return 0;
}
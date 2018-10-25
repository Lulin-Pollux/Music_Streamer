#include <stdio.h>
#include <Windows.h>


//�Է� ���۸� ����ش�.
void clearInputBuffer()
{
	while (getchar() != '\n');
}

//�ܼ��� ���ڻ��� �����Ѵ�.
int textcolor(unsigned short color_number)
{
	int retval = SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color_number);
	return retval;
}

//Ŀ���� ��ġ�� �����Ѵ�.
int setCursorPos(short x, short y)
{
	COORD position = { x - 1, y - 1 };
	int retval = SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
	return retval;
}

//ȭ�� ���� ũ�⸦ �����Ѵ�.
int setScreenBufferSize(short x, short y)
{
	COORD size = { x, y };
	int retval = SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), size);
	return retval;
}
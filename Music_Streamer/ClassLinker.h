/********************************************************************************
*                                                                               *
* ClassLinker.h -- Custom functions compilation                                 *
*                                                                               *
* Made by Lulin Pollux.          MIT License                                    *
*                                                                               *
********************************************************************************/


/* ��� �ڵ����Ͽ��� ���̴� ��ó����, ����ü ���� ���
--------------------------------------------------------------------*/
//���α׷��� ������ ������ ����ü
typedef struct settings
{
	char execute_mode[10];  //���α׷��� ���۸�带 ����
	char server_ip[16];  //������ IP�� ����
	unsigned int uid;  //�� ���α׷��� �⺻ Ű
	char nickName[100];  //������� �г���
}SETTINGS;

//���ڻ� ������ �� ���� ��� ���
#define RESET 7
#define DARK_BLUE 1
#define DARK_GREEN 2
#define BRIGHT_BLUE 3
#define DARK_RED 4
#define DARK_PURPLE 5
#define DARK_YELLOW 6
#define DARK_WHITE 7
#define GRAY 8
#define BLUE 9
#define GREEN 10
#define SKY_BLUE 11
#define RED 12
#define PURPLE 13
#define YELLOW 14
#define WHITE 15
/*--------------------------------------------------------------------*/


/* Server.c �� �Լ� ���
--------------------------------------------*/
int server();


/* Client.c �� �Լ� ���
--------------------------------------------*/
int client(SETTINGS *setUp);


/* Error.c �� �Լ� ��� 
--------------------------------------------*/
// ���� �Լ� ���� ��� �� ����
void err_quit(char *msg);

// ���� �Լ� ���� ���
void err_display(char *msg);


/* File.c �� �Լ� ���
--------------------------------------------*/
//������ �۽��ϴ� �Լ�
int fileSend(SOCKET sock, _In_ char *fileName);

//������ �����ϴ� �Լ�
int fileReceive(SOCKET sock, char *received_fileName, float *received_fileSize);

//���������� �ҷ����� �Լ�
int importSettings(SETTINGS *setUp);

//������ �����ο� �̸��� �Ѱ��ִ� �Լ�
int GetFilePath(char *filePath, int filePath_len, char *fileName);


/* Console.c �� �Լ� ���
--------------------------------------------*/
//�Է� ���۸� ����ش�.
void clearInputBuffer();

//�ܼ��� ���ڻ��� �����Ѵ�.
int textcolor(unsigned short color_number);

//Ŀ���� ��ġ�� �����Ѵ�.
int setCursorPos(short x, short y);

//ȭ�� ���� ũ�⸦ �����Ѵ�.
int setScreenBufferSize(short x, short y);
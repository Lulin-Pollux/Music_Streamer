/********************************************************************************
*                                                                               *
* ClassLinker.h -- Custom functions compilation                                 *
*                                                                               *
* Made by Lulin Pollux.          MIT License                                    *
*                                                                               *
********************************************************************************/


/* 모든 코드파일에서 쓰이는 전처리문, 구조체 등의 목록
--------------------------------------------------------------------*/
//프로그램의 설정을 저장할 구조체
typedef struct settings
{
	char execute_mode[7];		//프로그램의 시작모드
	char server_ip[16];			//서버의 IP 주소
	int server_mainPort;		//서버의 주 포트
	int server_requestPort;		//서버의 풀리퀘 포트
	int server_uid;				//서버의 아이디
	char server_nickName[50];	//서버의 닉네임
	int client_uid;				//클라이언트의 아이디
	char client_nickName[50];	//클라이언트의 닉네임
}SETTINGS;

//글자색 변경할 때 숫자 대신 사용
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


/* Client.c 의 함수 목록
--------------------------------------------*/
int client(SETTINGS sets);


/* Console.c 의 함수 목록
--------------------------------------------*/
//입력 버퍼를 비워준다.
void clearInputBuffer();

//콘솔의 글자색을 변경한다.
int textcolor(unsigned short color_number);

//커서의 위치를 변경한다.
int setCursorPos(short x, short y);

//화면 버퍼 크기를 설정한다.
int setScreenBufferSize(short x, short y);


/* Error.c 의 함수 목록
--------------------------------------------*/
// 소켓 함수 오류 출력 후 종료
void err_quit(char *msg);

// 소켓 함수 오류 출력
void err_display(char *msg);


/* File.c 의 함수 목록
--------------------------------------------*/
//파일 송신 함수 (경로는 최대 512바이트)
int sendFile(SOCKET sock, _In_ char *filePath, _Out_ double *sendByte);

//재생목록에 있는 모든 파일 전송 함수
int sendFullPlaylist(SOCKET sock, char playlist[][512]);

//파일 수신 함수 (경로는 최대 512바이트)
int recvFile(SOCKET sock, _Out_ char *filePath, _Out_ double *recvByte);

//재생목록에 있는 모든 파일 수신 함수
int recvFullPlayList(SOCKET sock, char playList[][512]);

//설정파일을 불러오는 함수
int importSettings(SETTINGS *sets);

//재생목록을 초기화하는 함수
int initializePlaylist(char playlist[][512]);


/* Player.c 의 함수 목록
--------------------------------------------*/
int player(char *fileName);


/* Server.c 의 함수 목록
--------------------------------------------*/
int server(SETTINGS sets);
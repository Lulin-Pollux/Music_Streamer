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

//재생목록 배열의 포인터
typedef char (*PLAYLIST_PTR)[512];

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
//클라이언트 메인 함수
int client(SETTINGS sets);

//서버에서 서버와 클라이언트의 아이디, 닉네임을 교환하는 함수
int client_exchangeIdNickname(SOCKET sock, SETTINGS *sets);

//재생목록을 새로고침하는 함수
int refreshPlaylist(SOCKET sock, char playlist[][512]);


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
int sendFile(SOCKET sock, _In_ char *filePath, _Out_ double *sendBytes);

//재생목록에 있는 모든 파일 전송 함수
int sendFullPlaylist(SOCKET sock, _In_ char playlist[][512], _Out_ double *allSendBytes);

//파일 수신 함수 (경로는 최대 512바이트)
int recvFile(SOCKET sock, _Out_ char *filePath, _Out_ double *recvBytes);

//재생목록에 있는 모든 파일 수신 함수
int recvFullPlayList(SOCKET sock, _Out_ char playlist[][512], _Out_ double *allRecvBytes);

//설정파일을 불러오는 함수
int importSettings(_Out_ SETTINGS *sets);

//재생목록을 초기화하는 함수
int initializePlaylist(_Out_ char playlist[][512]);


/* MCI Controller.c 의 함수 목록
--------------------------------------------*/
//MCI 오류를 출력하는 함수
int printMciError(int errorCode);

//각 모드에 알맞는 설명을 출력하는 함수
int printModeDescription(int mode);

//오디오 파일을 여는 함수
int openAudioFile(_Out_ MCIDEVICEID *deviceID, MCI_OPEN_PARMS mciOpenParms);

//오디오 파일을 닫는 함수
int closeAudioFile(MCIDEVICEID deviceID);

//오디오 파일을 재생하는 함수
int playAudioFile(MCIDEVICEID deviceID, MCI_PLAY_PARMS mciPlayParms);

//오디오 파일 재생을 일시정지하는 함수
//메모리에 오디오 파일을 유지시켜, 다시 재생할 때 빠르게 재생한다.
int pauseAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS mciGenericParms);

//오디오 파일 재생을 재개하는 함수
int resumeAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS mciGenericParms);

//오디오 파일 재생을 일시정지하는 함수
//메모리에서 오디오 파일을 내려버린다.
int stopAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS mciGenericParms);

//시간 형식을 설정하는 함수
int setTimeFormat(MCIDEVICEID deviceID, MCI_SET_PARMS mciSetParms, DWORD timeFormat);

//재생 위치를 이동시키는 함수 (단위: Miliseconds)
int seekAudioFile(MCIDEVICEID deviceID, MCI_SEEK_PARMS mciSeekParms, DWORD seekTo_ms);

//현재 상태를 가져오는 함수
int getCurrentMode(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms, _Out_ int *mode);

//재생할 파일의 총 길이를 가져오는 함수
int getAudioFileLength(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms, _Out_ unsigned int *totalLen_ms);

//현재 재생위치를 가져오는 함수
int getCurrentPosition(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms, _Out_ unsigned int *position_ms);


/* MusicPlayer.c 의 함수 목록
--------------------------------------------*/
//전체 재생목록을 출력한다.
int printFullPlaylist(char playlist[][512]);

//재생목록을 추가하는 함수
int insertPlaylist(char *fileName, char playlist[][512]);

//재생목록을 변경하는 함수
int updatePlaylist(char playlist[][512]);

//재생목록을 삭제하는 함수
int deletePlaylist(int row, char playlist[][512]);

//재생상태를 보여주는 함수
int showPlayStatus(MCIDEVICEID deviceID, MCI_STATUS_PARMS mciStatusParms);

//클라이언트용 음악 플레이어
int client_MusicPlayer(char *filePath);


/* Server.c 의 함수 목록
--------------------------------------------*/
//서버에서 서버와 클라이언트의 아이디, 닉네임을 교환하는 함수
int server_exchangeIdNickname(SOCKET sock, SETTINGS *sets);

//서버 메인 함수
int server(SETTINGS sets);
#include <stdio.h>
#include <Windows.h>
#include <Digitalv.h>
#include "ClassLinker.h"

#define SEEK_TO_START 0
#define SEEK_TO_END 1
#define SEEK_TO 2


//MCI ������ ����ϴ� �Լ�
int printMciError(int errorCode)
{
	int retval;

	char buffer[128];
	retval = mciGetErrorString(errorCode, buffer, sizeof(buffer));
	if (retval != TRUE)
	{
		printf("�ش� �����ڵ带 �� �� �����ϴ�. \n");
		return 1;
	}
	else
		printf("%s \n", buffer);

	return 0;
}

//����� ������ �ݴ� �Լ�
int closeAudioFile(MCIDEVICEID deviceID)
{
	int retval;

	//���� ��ġ ����̹� �ݱ�
	retval = mciSendCommand(deviceID, MCI_CLOSE, MCI_NOTIFY, 0);
	if (retval != 0)
		printMciError(retval);

	return 0;
}

//����� ������ ����ϴ� �Լ�
int playAudioFile(MCIDEVICEID deviceID, MCI_PLAY_PARMS *mciPlay)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mciPlay);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//����� ���� ����� �Ͻ������ϴ� �Լ�
//�޸𸮿� ����� ������ ��������, �ٽ������ �� ������ ����Ѵ�.
int pauseAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS *mciGeneric)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_PAUSE, MCI_NOTIFY, (DWORD_PTR)&mciGeneric);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//����� ���� ����� �簳�ϴ� �Լ�
int resumeAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS *mciGeneric)
{
	int retval;

	retval = mciSendCommand(deviceID, MCI_RESUME, MCI_NOTIFY, (DWORD_PTR)&mciGeneric);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//����� ���� ����� �����ϴ� �Լ�(mciGeneric = NULL)
//�Ͻ������� �ٸ��� �޸𸮿��� ����� ������ ����������.
int stopAudioFile(MCIDEVICEID deviceID, MCI_GENERIC_PARMS *mciGeneric)
{
	int retval;
	
	retval = mciSendCommand(deviceID, MCI_STOP, MCI_NOTIFY, (DWORD_PTR)&mciGeneric);
	if (retval != 0)
	{
		printMciError(retval);
		return 1;
	}

	return 0;
}

//��� ��ġ�� �̵���Ű�� �Լ�
int seekAudioFile(MCIDEVICEID deviceID, MCI_SEEK_PARMS *mciSeek, int seekControl)
{
	int retval;

	switch (seekControl)
	{
	//���� ��ġ�� �̵�
	case SEEK_TO_START:
		retval = mciSendCommand(deviceID, MCI_SEEK, MCI_SEEK_TO_START, (DWORD_PTR)&mciSeek);
		if (retval != 0)
		{
			printMciError(retval);
			return 1;
		}
		break;
	//�� ��ġ�� �̵�
	case SEEK_TO_END:
		retval = mciSendCommand(deviceID, MCI_SEEK, MCI_SEEK_TO_END, (DWORD_PTR)&mciSeek);
		if (retval != 0)
		{
			printMciError(retval);
			return 1;
		}
		break;
	//Ư�� ��ġ�� �̵�
	case SEEK_TO:
		retval = mciSendCommand(deviceID, MCI_SEEK, MCI_TO, (DWORD_PTR)&mciSeek);
		if (retval != 0)
		{
			printMciError(retval);
			return 1;
		}
		break;
	}

	return 0;
}
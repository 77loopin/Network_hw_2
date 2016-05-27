#pragma once
#include "socket_lib.h"

// �ʿ��� ��� ����
#define NICKBUFSIZE 100 // �г��� ���� ũ��


// ���� ���α׷��� �������� �����ϴ� ����ü ����
typedef struct Setup {
	char nickname[NICKBUFSIZE + 1]; // ������� ��ȭ���� �����ϴ� �������
	char mulchat_ip[40]; // ���� �������� ä�ù��� IP�� �����ϴ� �������
	int mulchat_port; // ���� �������� ä�ù��� Port�� �����ϴ� �������
	int connectFlag; // ���� ���� ���¸� ��Ÿ���� flag ��� ����
} setup;
//******* ConnectFlag�� ���ؼ� *********/
// 0 : �� ����
// 1 : ���� ����
// 2 : �г��� �ߺ� ���� (��� �޽����� ��µ�)


// �ʿ��� ���� ���� ����
extern HINSTANCE hInst; // WinMain�� �ν��Ͻ��� ����
extern CRITICAL_SECTION cs; // �Ӱ豸���� �����ϱ� ���� criticalSection ����
extern char user_ID[10]; // �г��� �̿ܿ� user�� �����ϱ� ���� 8byte id��
extern setup setting; // ���� ���α׷��� ���� ���¸� �����ϴ� ����ü
//extern message sendMsg; // ���� �޽����� �����ϴ� ����ü

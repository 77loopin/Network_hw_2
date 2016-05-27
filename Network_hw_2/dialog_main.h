#pragma once
#include "socket_lib.h"


// �ʿ��� ���� ���� ����
extern HWND hEdit1, hEdit2; // �޽��� �Է�, ��� �����͹ڽ� �ڵ鷯
extern HWND hMenu; // �޴� �ڵ鷯



// ��ȭ���� ���ν��� 
BOOL CALLBACK main_DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK setup_DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK nickname_DlgProc(HWND, UINT, WPARAM, LPARAM);


// �� ���� ����� ���� �Լ�
void DisplayText(char *fmt, ...); // �޽��� ��¿� ������ �ڽ��� ���¿� �µ��� �޽����� ����ϴ� �Լ�
BOOL check_ip(char* ip_addr, int len); // �Էµ� ��Ƽĳ��Ʈ IP�� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_port(char* port); // �Էµ� Port�� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_nick(char* nickname); // �Էµ� ��ȭ���� ��ȿ���� Ȯ���ϴ� �Լ�

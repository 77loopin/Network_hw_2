#pragma once
#include "socket_lib.h"


// 필요한 전역 변수 선언
extern HWND hEdit1, hEdit2; // 메시지 입력, 출력 에디터박스 핸들러
extern HWND hMenu; // 메뉴 핸들러



// 대화상자 프로시저 
BOOL CALLBACK main_DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK setup_DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK nickname_DlgProc(HWND, UINT, WPARAM, LPARAM);


// 그 외의 사용자 정의 함수
void DisplayText(char *fmt, ...); // 메시지 출력용 에디터 박스에 형태에 맞도록 메시지를 출력하는 함수
BOOL check_ip(char* ip_addr, int len); // 입력된 멀티캐스트 IP가 유효한지 확인하는 함수
BOOL check_port(char* port); // 입력된 Port가 유효한지 확인하는 함수
BOOL check_nick(char* nickname); // 입력된 대화명이 유효한지 확인하는 함수

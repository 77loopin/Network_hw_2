#pragma once
#include "socket_lib.h"

// 필요한 상수 정의
#define NICKBUFSIZE 100 // 닉네임 버퍼 크기


// 현재 프로그램의 설정값을 저장하는 구조체 정의
typedef struct Setup {
	char nickname[NICKBUFSIZE + 1]; // 사용자의 대화명을 저장하는 멤버변수
	char mulchat_ip[40]; // 현재 접속중인 채팅방의 IP를 저장하는 멤버변수
	int mulchat_port; // 현재 접속중인 채팅방의 Port를 저장하는 멤버변수
	int connectFlag; // 현재 접속 상태를 나타내는 flag 멤버 변수
} setup;
//******* ConnectFlag에 대해서 *********/
// 0 : 미 접속
// 1 : 정상 접속
// 2 : 닉네임 중복 상태 (경고 메시지가 출력됨)


// 필요한 전역 변수 선언
extern HINSTANCE hInst; // WinMain의 인스턴스를 저장
extern CRITICAL_SECTION cs; // 임계구역에 진입하기 위한 criticalSection 변수
extern char user_ID[10]; // 닉네임 이외에 user를 구분하기 위한 8byte id값
extern setup setting; // 현재 프로그램의 설정 상태를 저장하는 구조체
//extern message sendMsg; // 보낼 메시지를 저장하는 구조체

// 2016년 1학기 네트워크프로그램 숙제 2번
// 성명 : 백승재
// 학번 : 101969

#pragma comment(lib, "ws2_32")
#include "resource.h" // GUI 관련
#include "main.h" // 프로그램에 필요한 정보
#include "socket_multicast.h" // 소켓 관련
#include "dialog_main.h" // dialogbox 관련


// Dialog, thread, instance 핸들러 변수 선언
HINSTANCE hInst; // 현재 프로그램의 인스턴스 핸들을 저장하는 변수 선언
HWND hEdit1, hEdit2; // 메시지 입력, 출력 에디터 박스 선언
HWND hMenu; // 메뉴바 핸들러 선언
HANDLE hReceiver, hSender; // Receiver와 Sender의 핸들을 저장하는 변수 선언


// socket 관련 변수 선언
SOCKET sock_receiver; // recevier의 소켓 정보를 저장할 변수 선언
struct ip_mreq mreq_receiver; // receiver 멀티캐스트 정보를 저장할 변수 선언
WSADATA wsa_receiver; // receiver의 WinSock을 저장할 변수 선언


// 프로그램에 필요한 정보를 저장한 변수 선언
CRITICAL_SECTION cs; // critical section에 접근 하기 위한 CS 변수 선언
message sendMsg; // 보낼 메시지를 저장하는 구조체 선언
setup setting; // 프로그램의 접속 상태를 저장하는 구조체 선언
char user_ID[10]; // 사용자를 구분해 줄 ID값을 저장하는 변수 선언



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	// Store instance
	hInst = hInstance;

	// socket setting init
	strncpy(setting.mulchat_ip,DEFAULT_IP,strlen(DEFAULT_IP)+1);
	setting.mulchat_port = DEFAULT_PORT;
	setting.connectFlag = 0; // 초기 상태, 비접속

	// init critical section
	InitializeCriticalSection(&cs);

	// Allocate user ID
	srand(time(NULL));
	itoa(rand(),user_ID,16);

	// run DialogBox
	DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, main_DlgProc);

	// delete critical section
	DeleteCriticalSection(&cs);
	return 0;
}


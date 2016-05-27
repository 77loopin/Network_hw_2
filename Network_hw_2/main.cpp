// 2016�� 1�б� ��Ʈ��ũ���α׷� ���� 2��
// ���� : �����
// �й� : 101969

#pragma comment(lib, "ws2_32")
#include "resource.h" // GUI ����
#include "main.h" // ���α׷��� �ʿ��� ����
#include "socket_multicast.h" // ���� ����
#include "dialog_main.h" // dialogbox ����


// Dialog, thread, instance �ڵ鷯 ���� ����
HINSTANCE hInst; // ���� ���α׷��� �ν��Ͻ� �ڵ��� �����ϴ� ���� ����
HWND hEdit1, hEdit2; // �޽��� �Է�, ��� ������ �ڽ� ����
HWND hMenu; // �޴��� �ڵ鷯 ����
HANDLE hReceiver, hSender; // Receiver�� Sender�� �ڵ��� �����ϴ� ���� ����


// socket ���� ���� ����
SOCKET sock_receiver; // recevier�� ���� ������ ������ ���� ����
struct ip_mreq mreq_receiver; // receiver ��Ƽĳ��Ʈ ������ ������ ���� ����
WSADATA wsa_receiver; // receiver�� WinSock�� ������ ���� ����


// ���α׷��� �ʿ��� ������ ������ ���� ����
CRITICAL_SECTION cs; // critical section�� ���� �ϱ� ���� CS ���� ����
message sendMsg; // ���� �޽����� �����ϴ� ����ü ����
setup setting; // ���α׷��� ���� ���¸� �����ϴ� ����ü ����
char user_ID[10]; // ����ڸ� ������ �� ID���� �����ϴ� ���� ����



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	// Store instance
	hInst = hInstance;

	// socket setting init
	strncpy(setting.mulchat_ip,DEFAULT_IP,strlen(DEFAULT_IP)+1);
	setting.mulchat_port = DEFAULT_PORT;
	setting.connectFlag = 0; // �ʱ� ����, ������

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


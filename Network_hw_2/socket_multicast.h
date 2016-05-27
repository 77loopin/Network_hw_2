// ��Ƽĳ��Ʈ �Լ� ������ ���� �������
#include "socket_lib.h"
#include "dialog_main.h"
#include "main.h"


// �ʿ��� ��� ����
#define DEFAULT_PORT 9999 // multicast port
#define DEFAULTBUFSIZE 512 // ���� ũ��
#define DEFAULT_IP "224.0.0.1" // multicast ip
#define SOCKETBUFSIZE 512 // ���� ���� ũ��


// �޽��� ����ü ����
typedef struct Message {
	char id[10]; // ������� ID�� �����ϴ� �������
	char nickname[NICKBUFSIZE + 1]; // ������� ��ȭ���� �����ϴ� �������
	char data[SOCKETBUFSIZE + 1]; // ���� �޽���, Ȥ�� ���� �����͸� �����ϴ� �������
	int messageFlag; // ������ �޽����� ������ ��Ÿ���� flag ����
} message;
//******* messageFlag�� ���ؼ� *********/
// 1 : �������� ä�� �޽���
// 2 : ä�ù� ���ӽ� ��ȭ�� ���� �޽���
// 3 : ���� ��ȭ������ ������ ����ڿ��� ������ ��� �޽���
// 4 : ��ȭ�� ����� ��ȭ�� ���� �޽���


// �ʿ��� ���� ���� ����
extern SOCKET sock_receiver; // Receiver�� ���� ������ �����ϴ� ����
extern struct ip_mreq mreq_receiver; // Receiver�� ��Ƽĳ��Ʈ ������ �����ϴ� ����
extern WSADATA wsa_receiver; // Receiver�� WinSock ����
extern HANDLE hReceiver, hSender; // Receiver�� Sender�� ������ �ڵ��� �����ϴ� ����
extern message sendMsg; // ���� �޽����� �����ϴ� ����ü


// Thread�� ���� �� �Լ� ���� ����
DWORD WINAPI MulChat_Receiver(LPVOID arg); // Receiver
DWORD WINAPI MulChat_Ad_Sender(LPVOID arg); // Advertisement Sender
DWORD WINAPI MulChat_Msg_Sender(LPVOID arg); // Message Sender


// �� ���� ����� ���� �Լ�
struct tm* gettime(); // ���� �ð��� ���ؼ� tm ����ü�� �ּҰ��� ��ȯ�ϴ� �Լ�
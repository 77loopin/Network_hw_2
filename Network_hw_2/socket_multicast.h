// 멀티캐스트 함수 구현을 위한 헤더파일
#include "socket_lib.h"
#include "dialog_main.h"
#include "main.h"


// 필요한 상수 정의
#define DEFAULT_PORT 9999 // multicast port
#define DEFAULTBUFSIZE 512 // 버퍼 크기
#define DEFAULT_IP "224.0.0.1" // multicast ip
#define SOCKETBUFSIZE 512 // 소켓 버퍼 크기


// 메시지 구조체 정의
typedef struct Message {
	char id[10]; // 사용자의 ID를 저장하는 멤버변수
	char nickname[NICKBUFSIZE + 1]; // 사용자의 대화명을 저장하는 멤버변수
	char data[SOCKETBUFSIZE + 1]; // 보낼 메시지, 혹은 보낼 데이터를 저장하는 멤버변수
	int messageFlag; // 보내는 메시지의 종류를 나타내는 flag 변수
} message;
//******* messageFlag에 대해서 *********/
// 1 : 정상적인 채팅 메시지
// 2 : 채팅방 접속시 대화명 광고 메시지
// 3 : 같은 대화명으로 접속한 사용자에게 보내는 경고 메시지
// 4 : 대화명 변경시 대화명 광고 메시지


// 필요한 전역 변수 선언
extern SOCKET sock_receiver; // Receiver의 소켓 정보를 저장하는 변수
extern struct ip_mreq mreq_receiver; // Receiver의 멀티캐스트 정보를 저장하는 변수
extern WSADATA wsa_receiver; // Receiver의 WinSock 변수
extern HANDLE hReceiver, hSender; // Receiver와 Sender의 스레드 핸들을 저장하는 변수
extern message sendMsg; // 보낼 메시지를 저장하는 구조체


// Thread로 실행 할 함수 원형 선언
DWORD WINAPI MulChat_Receiver(LPVOID arg); // Receiver
DWORD WINAPI MulChat_Ad_Sender(LPVOID arg); // Advertisement Sender
DWORD WINAPI MulChat_Msg_Sender(LPVOID arg); // Message Sender


// 그 외의 사용자 정의 함수
struct tm* gettime(); // 현재 시간을 구해서 tm 구조체의 주소값을 반환하는 함수
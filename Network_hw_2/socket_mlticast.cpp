#include "socket_multicast.h"
#include "socket_error.h"
#include "socket_lib.h"
#include "main.h"


// 멀티캐스트 Receiver 함수
DWORD WINAPI MulChat_Receiver(LPVOID arg) {
	int retval; // return value
	BOOL sock_option; // socket option value
	char multicast_ip[40]; // 접속하는 멀티캐스트 ip를 저장하는 임시 변수
	char myNickName[NICKBUFSIZE + 1]; // 현재 대화명을 저장하는 임시 변수
	int multicast_port; // 접속하는 포트를 저장하는 임시 변수
	HANDLE hAdSender; // 광고 메시지를 보낼 Sender Thread 핸들을 저장할 임시 변수
	struct tm *timeinfo; // 현재 시간을 저장하는 tm 구조체 포인터 변수

	// multicast ip/port
	// setting 구조체를 참조하여 멀티캐스트 ip/port를 설정함 (Critical Section)
	EnterCriticalSection(&cs);
	strncpy(multicast_ip,setting.mulchat_ip,strlen(setting.mulchat_ip)+1);
	multicast_port = setting.mulchat_port;
	LeaveCriticalSection(&cs);

	// winsocket init
	//WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_receiver) != 0)
		return 1;
	
	// create socket
	sock_receiver = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_receiver == INVALID_SOCKET) err_quit("socket()");


	// set SO_REUSEADDR option
	sock_option = TRUE; // socket option value
	retval = setsockopt(sock_receiver, SOL_SOCKET, SO_REUSEADDR, (char*)&sock_option, sizeof(sock_option));
	
	// bind
	SOCKADDR_IN local; // localhost network information
	memset(&local, 0, sizeof(local)); // value init to 0
	
	// set local network information
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(setting.mulchat_port);
	retval = bind(sock_receiver, (SOCKADDR*)&local, sizeof(local));
	if (retval == SOCKET_ERROR) err_quit("bind()");


	// multicast group join
	mreq_receiver.imr_multiaddr.s_addr = inet_addr(setting.mulchat_ip);
	mreq_receiver.imr_interface.s_addr = htonl(INADDR_ANY);
	retval = setsockopt(sock_receiver, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		(char*)&mreq_receiver, sizeof(mreq_receiver));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	
	SOCKADDR_IN dest;
	int addrlen;
	message recvMsg;
	memset(&recvMsg, 0, sizeof(message));
	addrlen = sizeof(dest);

	// setting 구조체에서 대화명 값을 가져온다. (critical section)
	EnterCriticalSection(&cs);
	strncpy(myNickName, setting.nickname, NICKBUFSIZE + 1);
	LeaveCriticalSection(&cs);

	// Send Nickname Advertisement Message
	// 대화명을 광고하는 메시지를 전송한다.
	message AdMsg;
	AdMsg.messageFlag = 2;
	strncpy(AdMsg.nickname, myNickName, NICKBUFSIZE + 1);
	hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
	while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
	CloseHandle(hAdSender);

	// 접속한 내용을 화면에 출력
	timeinfo = gettime();
	DisplayText("[%2d:%02d | %s] : %s%s\r\n", timeinfo->tm_hour, timeinfo->tm_min, "info", myNickName, "님이 접속했습니다.");


	while (1) {
		retval = recvfrom(sock_receiver, (char*)&recvMsg, sizeof(message), 0, (SOCKADDR*)&dest, &addrlen);
		timeinfo = gettime(); // recv를 받은 시간을 저장
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		
		// recv message
		recvMsg.nickname[retval] = '\0';

		// setting 구조체에서 대화명과 현재 접속 상태를 확인해서 변수에 저장한다.
		EnterCriticalSection(&cs);
		strncpy(myNickName, setting.nickname, NICKBUFSIZE + 1);
		int myconnect = setting.connectFlag;
		LeaveCriticalSection(&cs);


		// 받은 메시지의 상태별로 메시지 처리
		if (recvMsg.messageFlag == 1) { // 받은 메시지가 일반 채팅 메시지라면
			// 받은 메시지 출력
			DisplayText("[%02d:%02d | %s] : %s\r\n", timeinfo->tm_hour, timeinfo->tm_min, recvMsg.nickname, recvMsg.data);

			// 메시지를 보낸 사용자의 대화명이 나의 대화명과 동일하고 내가 먼저 접속했고 또 ID가 다르면 (다른사용자라는 뜻)
			if (!strcmp(recvMsg.nickname, myNickName) && myconnect != 2 && strcmp(recvMsg.id, user_ID)) {
				// 경고 메시지를 전송한다.
				message AdMsg;
				AdMsg.messageFlag = 3;
				strncpy(AdMsg.nickname, myNickName, NICKBUFSIZE + 1);
				hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				CloseHandle(hAdSender);
			}
		}
		else if ((recvMsg.messageFlag == 2 || recvMsg.messageFlag == 4) && strcmp(recvMsg.id,user_ID)) { // 다른 사용자의 광고 메시지를 받음
			// 광고 메시지 처리
			if (recvMsg.messageFlag == 2) // 받은 메시지가 접속 광고 메시지
				DisplayText("[%02d:%02d | %s] : %s%s\r\n",timeinfo->tm_hour,timeinfo->tm_min,"info", recvMsg.nickname, "님이 접속했습니다.");
			else  // 받은 메시지가 대화명 변경 광고 메시지
				DisplayText("[%02d:%02d | %s] : %s -> %s %s\r\n", timeinfo->tm_hour, timeinfo->tm_min, "info", recvMsg.data, recvMsg.nickname, " 닉네임을 변경했습니다.");

			// 광고 메시지의 대화명이 나와 같다면
			if (!strcmp(recvMsg.nickname, myNickName)) {
				// 경고 메시지 전송
				message AdMsg;
				AdMsg.messageFlag = 3;
				strncpy(AdMsg.nickname, myNickName, NICKBUFSIZE + 1);
				hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				CloseHandle(hAdSender);
			}
		}
		else if (recvMsg.messageFlag == 3 && strcmp(recvMsg.id, user_ID)) { // 다른 사용자가 나에게 경고 메시지를 전달한 경우 (내가 늦게 접속한 경우)
			if (!strcmp(recvMsg.nickname, myNickName)) {
				// 나에게 경고 메시지를 출력
				DisplayText("<<%s>> %s", "경고", "이미 닉네임을 사용중입니다. 닉네임을 변경하세요.\n");
				EnterCriticalSection(&cs);
				setting.connectFlag = 2; // 접속 상태를 대화명 중복 상태로 설정
				LeaveCriticalSection(&cs);
			}
		}
	}
	return 0;
}

// 멀티캐스트 Advertisement Sender 함수
DWORD WINAPI MulChat_Ad_Sender(LPVOID arg) {
	int retval; // return value
	BOOL sock_option; // socket option value
	message SendMsg; // 보낼 메시지를 저장할 임시 메시지 구조체 변수
	char multicast_ip[40]; // ip 주소를 저장할 임시 변수
	char myNickName[NICKBUFSIZE + 1]; // 대화명을 저장할 임시 변수
	int multicast_port; // 포트를 저장할 임시 변수

	// multicast ip/port/nickname argument
	SendMsg = *(message*)arg; // 인자로 넘어온 메시지 변수를 저장
	
	// setting 구조체에서 접속 정보를 가져옴 ( critical section )
	EnterCriticalSection(&cs);
	strncpy(multicast_ip, setting.mulchat_ip, strlen(setting.mulchat_ip)+1);
	strncpy(myNickName, setting.nickname, NICKBUFSIZE + 1);
	multicast_port = setting.mulchat_port;
	strncpy(SendMsg.id, user_ID, 10);
	LeaveCriticalSection(&cs);
	
	// socket structure init
	SOCKADDR_IN dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(multicast_ip);
	dest.sin_port = htons(multicast_port);

	// init winsock
	WSADATA wsa_sender;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_sender) != 0)
		return 1;

	// socket()
	SOCKET sock_sender = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_sender == INVALID_SOCKET)	err_quit("socket()");

	// multicast ttl 2
	int ttl = 2;
	retval = setsockopt(sock_sender, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	//user_ID값을 메시지 구조체에 저장
	strncpy(SendMsg.id, user_ID,10);
	
	// 광고 메시지 전송
	retval = sendto(sock_sender, (char*)&SendMsg, sizeof(SendMsg), 0, (SOCKADDR*)&dest, sizeof(SendMsg));
	if (retval == SOCKET_ERROR) err_display("sendto()");

	closesocket(sock_sender);
	WSACleanup();
	return 0;
	
}

// 멀티캐스트 Message Sender 함수
DWORD WINAPI MulChat_Msg_Sender(LPVOID arg) {
	int retval; // return value
	BOOL sock_option; // socket option value
	message SendMsg; // 보낼 메시지를 저장할 임시 메시지 구조체 변수
	char multicast_ip[40]; // ip 주소를 저장할 임시 변수
	char myNickName[NICKBUFSIZE + 1]; // 대화명을 저장할 임시 변수
	int multicast_port; // 포트를 저장할 임시 변수


	// setting 구조체에서 접속 정보를 가져옴 ( critical section )
	EnterCriticalSection(&cs);
	SendMsg = sendMsg;
	strncpy(multicast_ip, setting.mulchat_ip, strlen(setting.mulchat_ip)+1);
	strncpy(myNickName, setting.nickname, NICKBUFSIZE + 1);
	multicast_port = setting.mulchat_port;
	strncpy(SendMsg.id, user_ID, 10);
	LeaveCriticalSection(&cs);

	// socket structure init
	SOCKADDR_IN dest;
	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = inet_addr(multicast_ip);
	dest.sin_port = htons(multicast_port);
	
	// init winsock
	WSADATA wsa_sender;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_sender) != 0)
		return 1;

	// socket()
	SOCKET sock_sender = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_sender == INVALID_SOCKET)	err_quit("socket()");

	// multicast ttl 2
	int ttl = 2;
	retval = setsockopt(sock_sender, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl));
	if (retval == SOCKET_ERROR) err_quit("setsockopt()");

	//user_ID값을 메시지 구조체에 저장
	strncpy(SendMsg.id, user_ID, 10);

	// 광고 메시지 전송
	retval = sendto(sock_sender, (char*)&SendMsg, sizeof(SendMsg), 0, (SOCKADDR*)&dest, sizeof(SendMsg));
	if (retval == SOCKET_ERROR) err_display("sendto()");

	closesocket(sock_sender);
	WSACleanup();
	return 0;

}

//현재 시간을 가져오는 함수
struct tm* gettime() {
	time_t timer;
	timer = time(NULL);
	return localtime(&timer);
}
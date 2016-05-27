#include "socket_multicast.h"
#include "socket_error.h"
#include "socket_lib.h"
#include "main.h"


// ��Ƽĳ��Ʈ Receiver �Լ�
DWORD WINAPI MulChat_Receiver(LPVOID arg) {
	int retval; // return value
	BOOL sock_option; // socket option value
	char multicast_ip[40]; // �����ϴ� ��Ƽĳ��Ʈ ip�� �����ϴ� �ӽ� ����
	char myNickName[NICKBUFSIZE + 1]; // ���� ��ȭ���� �����ϴ� �ӽ� ����
	int multicast_port; // �����ϴ� ��Ʈ�� �����ϴ� �ӽ� ����
	HANDLE hAdSender; // ���� �޽����� ���� Sender Thread �ڵ��� ������ �ӽ� ����
	struct tm *timeinfo; // ���� �ð��� �����ϴ� tm ����ü ������ ����

	// multicast ip/port
	// setting ����ü�� �����Ͽ� ��Ƽĳ��Ʈ ip/port�� ������ (Critical Section)
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

	// setting ����ü���� ��ȭ�� ���� �����´�. (critical section)
	EnterCriticalSection(&cs);
	strncpy(myNickName, setting.nickname, NICKBUFSIZE + 1);
	LeaveCriticalSection(&cs);

	// Send Nickname Advertisement Message
	// ��ȭ���� �����ϴ� �޽����� �����Ѵ�.
	message AdMsg;
	AdMsg.messageFlag = 2;
	strncpy(AdMsg.nickname, myNickName, NICKBUFSIZE + 1);
	hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
	while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
	CloseHandle(hAdSender);

	// ������ ������ ȭ�鿡 ���
	timeinfo = gettime();
	DisplayText("[%2d:%02d | %s] : %s%s\r\n", timeinfo->tm_hour, timeinfo->tm_min, "info", myNickName, "���� �����߽��ϴ�.");


	while (1) {
		retval = recvfrom(sock_receiver, (char*)&recvMsg, sizeof(message), 0, (SOCKADDR*)&dest, &addrlen);
		timeinfo = gettime(); // recv�� ���� �ð��� ����
		if (retval == SOCKET_ERROR) {
			err_display("recvfrom()");
			continue;
		}
		
		// recv message
		recvMsg.nickname[retval] = '\0';

		// setting ����ü���� ��ȭ��� ���� ���� ���¸� Ȯ���ؼ� ������ �����Ѵ�.
		EnterCriticalSection(&cs);
		strncpy(myNickName, setting.nickname, NICKBUFSIZE + 1);
		int myconnect = setting.connectFlag;
		LeaveCriticalSection(&cs);


		// ���� �޽����� ���º��� �޽��� ó��
		if (recvMsg.messageFlag == 1) { // ���� �޽����� �Ϲ� ä�� �޽������
			// ���� �޽��� ���
			DisplayText("[%02d:%02d | %s] : %s\r\n", timeinfo->tm_hour, timeinfo->tm_min, recvMsg.nickname, recvMsg.data);

			// �޽����� ���� ������� ��ȭ���� ���� ��ȭ��� �����ϰ� ���� ���� �����߰� �� ID�� �ٸ��� (�ٸ�����ڶ�� ��)
			if (!strcmp(recvMsg.nickname, myNickName) && myconnect != 2 && strcmp(recvMsg.id, user_ID)) {
				// ��� �޽����� �����Ѵ�.
				message AdMsg;
				AdMsg.messageFlag = 3;
				strncpy(AdMsg.nickname, myNickName, NICKBUFSIZE + 1);
				hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				CloseHandle(hAdSender);
			}
		}
		else if ((recvMsg.messageFlag == 2 || recvMsg.messageFlag == 4) && strcmp(recvMsg.id,user_ID)) { // �ٸ� ������� ���� �޽����� ����
			// ���� �޽��� ó��
			if (recvMsg.messageFlag == 2) // ���� �޽����� ���� ���� �޽���
				DisplayText("[%02d:%02d | %s] : %s%s\r\n",timeinfo->tm_hour,timeinfo->tm_min,"info", recvMsg.nickname, "���� �����߽��ϴ�.");
			else  // ���� �޽����� ��ȭ�� ���� ���� �޽���
				DisplayText("[%02d:%02d | %s] : %s -> %s %s\r\n", timeinfo->tm_hour, timeinfo->tm_min, "info", recvMsg.data, recvMsg.nickname, " �г����� �����߽��ϴ�.");

			// ���� �޽����� ��ȭ���� ���� ���ٸ�
			if (!strcmp(recvMsg.nickname, myNickName)) {
				// ��� �޽��� ����
				message AdMsg;
				AdMsg.messageFlag = 3;
				strncpy(AdMsg.nickname, myNickName, NICKBUFSIZE + 1);
				hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
				CloseHandle(hAdSender);
			}
		}
		else if (recvMsg.messageFlag == 3 && strcmp(recvMsg.id, user_ID)) { // �ٸ� ����ڰ� ������ ��� �޽����� ������ ��� (���� �ʰ� ������ ���)
			if (!strcmp(recvMsg.nickname, myNickName)) {
				// ������ ��� �޽����� ���
				DisplayText("<<%s>> %s", "���", "�̹� �г����� ������Դϴ�. �г����� �����ϼ���.\n");
				EnterCriticalSection(&cs);
				setting.connectFlag = 2; // ���� ���¸� ��ȭ�� �ߺ� ���·� ����
				LeaveCriticalSection(&cs);
			}
		}
	}
	return 0;
}

// ��Ƽĳ��Ʈ Advertisement Sender �Լ�
DWORD WINAPI MulChat_Ad_Sender(LPVOID arg) {
	int retval; // return value
	BOOL sock_option; // socket option value
	message SendMsg; // ���� �޽����� ������ �ӽ� �޽��� ����ü ����
	char multicast_ip[40]; // ip �ּҸ� ������ �ӽ� ����
	char myNickName[NICKBUFSIZE + 1]; // ��ȭ���� ������ �ӽ� ����
	int multicast_port; // ��Ʈ�� ������ �ӽ� ����

	// multicast ip/port/nickname argument
	SendMsg = *(message*)arg; // ���ڷ� �Ѿ�� �޽��� ������ ����
	
	// setting ����ü���� ���� ������ ������ ( critical section )
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

	//user_ID���� �޽��� ����ü�� ����
	strncpy(SendMsg.id, user_ID,10);
	
	// ���� �޽��� ����
	retval = sendto(sock_sender, (char*)&SendMsg, sizeof(SendMsg), 0, (SOCKADDR*)&dest, sizeof(SendMsg));
	if (retval == SOCKET_ERROR) err_display("sendto()");

	closesocket(sock_sender);
	WSACleanup();
	return 0;
	
}

// ��Ƽĳ��Ʈ Message Sender �Լ�
DWORD WINAPI MulChat_Msg_Sender(LPVOID arg) {
	int retval; // return value
	BOOL sock_option; // socket option value
	message SendMsg; // ���� �޽����� ������ �ӽ� �޽��� ����ü ����
	char multicast_ip[40]; // ip �ּҸ� ������ �ӽ� ����
	char myNickName[NICKBUFSIZE + 1]; // ��ȭ���� ������ �ӽ� ����
	int multicast_port; // ��Ʈ�� ������ �ӽ� ����


	// setting ����ü���� ���� ������ ������ ( critical section )
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

	//user_ID���� �޽��� ����ü�� ����
	strncpy(SendMsg.id, user_ID, 10);

	// ���� �޽��� ����
	retval = sendto(sock_sender, (char*)&SendMsg, sizeof(SendMsg), 0, (SOCKADDR*)&dest, sizeof(SendMsg));
	if (retval == SOCKET_ERROR) err_display("sendto()");

	closesocket(sock_sender);
	WSACleanup();
	return 0;

}

//���� �ð��� �������� �Լ�
struct tm* gettime() {
	time_t timer;
	timer = time(NULL);
	return localtime(&timer);
}
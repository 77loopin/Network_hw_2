#include "resource.h"
#include "main.h"
#include "socket_multicast.h"
#include "socket_error.h"


// IP,Port,대화명을 입력 받기 위한 에디터 컨트롤 핸들러
HWND hIp, hPort, hNick;

// Setup DialogBox
BOOL CALLBACK setup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char ip[40]; // 입력받은 ip를 저장하는 임시 변수
	char port[6]; // 입력받은 port를 저장하는 임시 변수
	char Nick[NICKBUFSIZE + 1]; // 입력받은 대화명을 저장하는 임시 변수
	int iplen; // ip 문자열의 길이를 저장하는 변수
	int myConnect; // 현재 접속 상태를 저장하는 임시 변수
	switch (uMsg) {
	case WM_INITDIALOG: // DialogBox를 처음 실행 됐을 때 초기화
		hIp = GetDlgItem(hDlg, IDC_IPADDRESS1); // IDC_IPADDRESS1의 핸들을 가져온다.
		hPort = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1의 핸들을 가져온다.
		hNick = GetDlgItem(hDlg, IDC_EDIT2); // IDC_EDIT2의 핸들을 가져온다.
		SendMessage(hPort, EM_SETLIMITTEXT, 5, 0); // port의 최대 입력 크기를 5로 설정한다. (최대 65535이므로)
		SendMessage(hNick, EM_SETLIMITTEXT, NICKBUFSIZE, 0); // 대화명 입력의 최대 크기를 설정한다.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // 확인 버튼을 누른 경우 호출
			iplen = GetWindowTextLength(hIp); // IP의 길이를 리턴
			GetDlgItemText(hDlg, IDC_IPADDRESS1, ip, iplen+1); // IP를 입력 받음
			GetDlgItemText(hDlg, IDC_EDIT1, port, 6); // Port를 입력 받음
			GetDlgItemText(hDlg, IDC_EDIT2, Nick, NICKBUFSIZE + 1); // 대화명을 입력 받음
			

			// 입력된 ip,port,대화명이 유효한지 확인

			if (!check_ip(ip, iplen)) { // IP가 유효한 멀티캐스트 IP인지 확인
				// 유효한 IP가 아닌 경우 경고 메시지 호출 후 다시 입력 받도록 함
				MessageBox(hDlg, "IP가 Multicast IP가 아닙니다.\nIP를 올바르게 입력하세요.", "경고", MB_OK);
				SetFocus(hIp);
				return FALSE;
			}
			if (!check_port(port)) { // Port가 유효한지 확인
				// 유효한 Port가 아닌 경우 경고 메시지 호출 후 다시 입력 받도록 함
				MessageBox(hDlg, "PORT를 알 수 없습니다.\nPORT를 올바르게 입력하세요.\n\n[PORT는 1024 ~ 65535까지 가능]", "경고", MB_OK);
				SetFocus(hPort);
				return FALSE;
			}
			if (!check_nick(Nick)) { // 대화명을 입력 했는지 확인
				// 대화명을 입력하지 않았으면 경고 메시지 호출 후 다시 입력 받도록 함
				MessageBox(hDlg, "NickName을 입력하세요.", "경고", MB_OK);
				SetFocus(hNick);
				return FALSE;
			}


			// 입력된 채팅방 설정값을 setting 구조체에 저장 ( critical section )
			EnterCriticalSection(&cs);
			strncpy(setting.mulchat_ip, ip, 40);
			setting.mulchat_port = atoi(port);
			strncpy(setting.nickname, Nick, NICKBUFSIZE+1);
			myConnect = setting.connectFlag;
			setting.connectFlag = 1;
			LeaveCriticalSection(&cs);
			// leave critical section
		
			if (myConnect == 0) { // 현재 접속이 최초 접속이라면
				// 바로 Receiver Thread 생성
				hReceiver = NULL;
				while (hReceiver == NULL)
					hReceiver = CreateThread(NULL, 0, MulChat_Receiver, NULL, 0, NULL);
			}
			else if (myConnect == 1 || myConnect == 2) { // 설정을 변경하는 거라면
				int retval;
				TerminateThread(hReceiver, 0); // 기존에 실행되고 있던 Receiver Thread 강제 종료
				retval = setsockopt(sock_receiver, IPPROTO_IP, IP_DROP_MEMBERSHIP, // 현재 멀티캐스트 종료
					(char*)&mreq_receiver, sizeof(mreq_receiver));
				if (retval == SOCKET_ERROR) err_quit("setsockopt()");
				closesocket(sock_receiver); // 소켓을 닫는다.
				WSACleanup(); // WINSOCK을 닫는다.
				hReceiver = NULL; // 새로운 설정값으로 Receiver Trhead 생성
				while (hReceiver == NULL) 
					hReceiver = CreateThread(NULL, 0, MulChat_Receiver, NULL, 0, NULL);
			}
			EndDialog(hDlg, IDCANCEL); // configuration dialog box finish
			return TRUE;
		case IDCANCEL:
			if (setting.connectFlag == 0) { // 최초 접속 시 setting값이 없으면 프로그램 강제 종료
				MessageBox(hDlg, "접속한 채팅 방이 없습니다.\n프로그램을 종료합니다.", "종료", MB_OK);
				exit(0);
			}
			EndDialog(hDlg, IDCANCEL); // 설정을 변경하지 않으면 대화상자 종료
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// Nickname Setting DialogBox
BOOL CALLBACK nickname_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char newNick[NICKBUFSIZE + 1]; // 새로운 대화명을 저장하는 임시 변수
	char oldNick[NICKBUFSIZE + 1]; // 기존 대화명을 저장하는 임시 변수
	HWND hEdit;
	HANDLE hAdSender;
	struct tm* timeinfo;

	switch (uMsg) {
	case WM_INITDIALOG: // 프로그램을 처음 실행 했을 때 초기화
		hEdit = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1의 핸들을 가져온다.
		SendMessage(hEdit, EM_SETLIMITTEXT, NICKBUFSIZE, 0); // 대화명의 최대 입력 크기를 설정한다.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // 확인 버튼을 누른 경우 호출
			GetDlgItemText(hDlg, IDC_EDIT1, newNick, NICKBUFSIZE + 1); // IDC_EDIT1의 문자열을 newNick에 저장
			if (!check_nick(newNick)) { // 새로 입력된 대화명이 유효한지 확인
				MessageBox(hDlg, "NickName을 입력하세요.", "경고", MB_OK);
				SetFocus(hEdit);
				return FALSE;
			}

			// 새로운 대화명을 변경하기 위해 Setting 구조체에 접근 ( Critical section )
			EnterCriticalSection(&cs);
			strncpy(oldNick, setting.nickname, NICKBUFSIZE + 1);
			strncpy(setting.nickname, newNick, NICKBUFSIZE + 1);
			setting.connectFlag = 1; // 대화명을 바꿨으므로 정상 접속으로 설정
			LeaveCriticalSection(&cs);
			
			// 변경된 대화명으로 광고 메시지 전송
			message AdMsg;
			AdMsg.messageFlag = 4; // 대화명 변경 광고 메시지
			strncpy(AdMsg.data, oldNick, NICKBUFSIZE+1);
			strncpy(AdMsg.nickname, newNick, NICKBUFSIZE + 1);
			hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
			while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
			CloseHandle(hAdSender);

			// 대화명을 변경 했다는 것을 화면에 출력
			timeinfo = gettime();
			DisplayText("[%02d:%02d | %s] : %s -> %s %s\r\n", timeinfo->tm_hour, timeinfo->tm_min,"info", oldNick,newNick, " 닉네임을 변경했습니다.");
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료
			return TRUE;

		case IDCANCEL: // 취소 버튼을 누른 경우 호출
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료
			return TRUE;
		}
	}
	return FALSE;
}

// ip가 유효한 멀티캐스트 ip인지 확인하는 함수
BOOL check_ip(char* ip_addr, int len) {
	char tempString[40];
	char *ptr;
	int t = 0;
	strncpy(tempString, ip_addr, len+1);

	ptr = strtok(tempString, ".");
	t = atoi(ptr);
	if (t > 239 || t < 224) { // 첫번재 IP주소가 Multicast 주소(Class D)가 아닌 경우 오류
		return FALSE;
	}

	for (int i = 0; i < 2; i++) {
		strtok(NULL, ".");
	}

	ptr = strtok(NULL, ".");
	t = atoi(ptr);
	if (t == 0 || t == 255) // IP주소의 마지막값이 0 또는 255이면 주소값 오류
		return FALSE;

	return TRUE;
}

// 입력된 port 번호가 유효한지 확인하는 함수
BOOL check_port(char* port) {
	int t;
	for (int i = 0; i < strlen(port); i++) {
		if (port[i] > 0x39 || port[i] < 0x30) // 포트번호가 숫자값이 아니면 오류
			return FALSE;
	}

	t = atoi(port);

	if (t > 65535 || t < 1024) // 포트번호가 포트 숫자 범위 밖이거나 1024보다 작으면 오류
		return FALSE;
	return TRUE;
}

// 입력된 대화명이 유효한지 확인하는 함수
BOOL check_nick(char* nickname) {
	if (strlen(nickname) == 0) // 대화명 길이가 0이면 오류
		return FALSE;
	if (nickname[0] == '\n' || nickname[0] == '\r') // 대화명 첫 글자가 개행문자면 오류
		return FALSE;
	return TRUE;
}	
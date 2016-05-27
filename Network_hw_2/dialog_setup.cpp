#include "resource.h"
#include "main.h"
#include "socket_multicast.h"
#include "socket_error.h"


// IP,Port,��ȭ���� �Է� �ޱ� ���� ������ ��Ʈ�� �ڵ鷯
HWND hIp, hPort, hNick;

// Setup DialogBox
BOOL CALLBACK setup_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char ip[40]; // �Է¹��� ip�� �����ϴ� �ӽ� ����
	char port[6]; // �Է¹��� port�� �����ϴ� �ӽ� ����
	char Nick[NICKBUFSIZE + 1]; // �Է¹��� ��ȭ���� �����ϴ� �ӽ� ����
	int iplen; // ip ���ڿ��� ���̸� �����ϴ� ����
	int myConnect; // ���� ���� ���¸� �����ϴ� �ӽ� ����
	switch (uMsg) {
	case WM_INITDIALOG: // DialogBox�� ó�� ���� ���� �� �ʱ�ȭ
		hIp = GetDlgItem(hDlg, IDC_IPADDRESS1); // IDC_IPADDRESS1�� �ڵ��� �����´�.
		hPort = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1�� �ڵ��� �����´�.
		hNick = GetDlgItem(hDlg, IDC_EDIT2); // IDC_EDIT2�� �ڵ��� �����´�.
		SendMessage(hPort, EM_SETLIMITTEXT, 5, 0); // port�� �ִ� �Է� ũ�⸦ 5�� �����Ѵ�. (�ִ� 65535�̹Ƿ�)
		SendMessage(hNick, EM_SETLIMITTEXT, NICKBUFSIZE, 0); // ��ȭ�� �Է��� �ִ� ũ�⸦ �����Ѵ�.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // Ȯ�� ��ư�� ���� ��� ȣ��
			iplen = GetWindowTextLength(hIp); // IP�� ���̸� ����
			GetDlgItemText(hDlg, IDC_IPADDRESS1, ip, iplen+1); // IP�� �Է� ����
			GetDlgItemText(hDlg, IDC_EDIT1, port, 6); // Port�� �Է� ����
			GetDlgItemText(hDlg, IDC_EDIT2, Nick, NICKBUFSIZE + 1); // ��ȭ���� �Է� ����
			

			// �Էµ� ip,port,��ȭ���� ��ȿ���� Ȯ��

			if (!check_ip(ip, iplen)) { // IP�� ��ȿ�� ��Ƽĳ��Ʈ IP���� Ȯ��
				// ��ȿ�� IP�� �ƴ� ��� ��� �޽��� ȣ�� �� �ٽ� �Է� �޵��� ��
				MessageBox(hDlg, "IP�� Multicast IP�� �ƴմϴ�.\nIP�� �ùٸ��� �Է��ϼ���.", "���", MB_OK);
				SetFocus(hIp);
				return FALSE;
			}
			if (!check_port(port)) { // Port�� ��ȿ���� Ȯ��
				// ��ȿ�� Port�� �ƴ� ��� ��� �޽��� ȣ�� �� �ٽ� �Է� �޵��� ��
				MessageBox(hDlg, "PORT�� �� �� �����ϴ�.\nPORT�� �ùٸ��� �Է��ϼ���.\n\n[PORT�� 1024 ~ 65535���� ����]", "���", MB_OK);
				SetFocus(hPort);
				return FALSE;
			}
			if (!check_nick(Nick)) { // ��ȭ���� �Է� �ߴ��� Ȯ��
				// ��ȭ���� �Է����� �ʾ����� ��� �޽��� ȣ�� �� �ٽ� �Է� �޵��� ��
				MessageBox(hDlg, "NickName�� �Է��ϼ���.", "���", MB_OK);
				SetFocus(hNick);
				return FALSE;
			}


			// �Էµ� ä�ù� �������� setting ����ü�� ���� ( critical section )
			EnterCriticalSection(&cs);
			strncpy(setting.mulchat_ip, ip, 40);
			setting.mulchat_port = atoi(port);
			strncpy(setting.nickname, Nick, NICKBUFSIZE+1);
			myConnect = setting.connectFlag;
			setting.connectFlag = 1;
			LeaveCriticalSection(&cs);
			// leave critical section
		
			if (myConnect == 0) { // ���� ������ ���� �����̶��
				// �ٷ� Receiver Thread ����
				hReceiver = NULL;
				while (hReceiver == NULL)
					hReceiver = CreateThread(NULL, 0, MulChat_Receiver, NULL, 0, NULL);
			}
			else if (myConnect == 1 || myConnect == 2) { // ������ �����ϴ� �Ŷ��
				int retval;
				TerminateThread(hReceiver, 0); // ������ ����ǰ� �ִ� Receiver Thread ���� ����
				retval = setsockopt(sock_receiver, IPPROTO_IP, IP_DROP_MEMBERSHIP, // ���� ��Ƽĳ��Ʈ ����
					(char*)&mreq_receiver, sizeof(mreq_receiver));
				if (retval == SOCKET_ERROR) err_quit("setsockopt()");
				closesocket(sock_receiver); // ������ �ݴ´�.
				WSACleanup(); // WINSOCK�� �ݴ´�.
				hReceiver = NULL; // ���ο� ���������� Receiver Trhead ����
				while (hReceiver == NULL) 
					hReceiver = CreateThread(NULL, 0, MulChat_Receiver, NULL, 0, NULL);
			}
			EndDialog(hDlg, IDCANCEL); // configuration dialog box finish
			return TRUE;
		case IDCANCEL:
			if (setting.connectFlag == 0) { // ���� ���� �� setting���� ������ ���α׷� ���� ����
				MessageBox(hDlg, "������ ä�� ���� �����ϴ�.\n���α׷��� �����մϴ�.", "����", MB_OK);
				exit(0);
			}
			EndDialog(hDlg, IDCANCEL); // ������ �������� ������ ��ȭ���� ����
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

// Nickname Setting DialogBox
BOOL CALLBACK nickname_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char newNick[NICKBUFSIZE + 1]; // ���ο� ��ȭ���� �����ϴ� �ӽ� ����
	char oldNick[NICKBUFSIZE + 1]; // ���� ��ȭ���� �����ϴ� �ӽ� ����
	HWND hEdit;
	HANDLE hAdSender;
	struct tm* timeinfo;

	switch (uMsg) {
	case WM_INITDIALOG: // ���α׷��� ó�� ���� ���� �� �ʱ�ȭ
		hEdit = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1�� �ڵ��� �����´�.
		SendMessage(hEdit, EM_SETLIMITTEXT, NICKBUFSIZE, 0); // ��ȭ���� �ִ� �Է� ũ�⸦ �����Ѵ�.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // Ȯ�� ��ư�� ���� ��� ȣ��
			GetDlgItemText(hDlg, IDC_EDIT1, newNick, NICKBUFSIZE + 1); // IDC_EDIT1�� ���ڿ��� newNick�� ����
			if (!check_nick(newNick)) { // ���� �Էµ� ��ȭ���� ��ȿ���� Ȯ��
				MessageBox(hDlg, "NickName�� �Է��ϼ���.", "���", MB_OK);
				SetFocus(hEdit);
				return FALSE;
			}

			// ���ο� ��ȭ���� �����ϱ� ���� Setting ����ü�� ���� ( Critical section )
			EnterCriticalSection(&cs);
			strncpy(oldNick, setting.nickname, NICKBUFSIZE + 1);
			strncpy(setting.nickname, newNick, NICKBUFSIZE + 1);
			setting.connectFlag = 1; // ��ȭ���� �ٲ����Ƿ� ���� �������� ����
			LeaveCriticalSection(&cs);
			
			// ����� ��ȭ������ ���� �޽��� ����
			message AdMsg;
			AdMsg.messageFlag = 4; // ��ȭ�� ���� ���� �޽���
			strncpy(AdMsg.data, oldNick, NICKBUFSIZE+1);
			strncpy(AdMsg.nickname, newNick, NICKBUFSIZE + 1);
			hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
			while (hAdSender == NULL) hAdSender = CreateThread(NULL, 0, MulChat_Ad_Sender, &AdMsg, 0, NULL);
			CloseHandle(hAdSender);

			// ��ȭ���� ���� �ߴٴ� ���� ȭ�鿡 ���
			timeinfo = gettime();
			DisplayText("[%02d:%02d | %s] : %s -> %s %s\r\n", timeinfo->tm_hour, timeinfo->tm_min,"info", oldNick,newNick, " �г����� �����߽��ϴ�.");
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����
			return TRUE;

		case IDCANCEL: // ��� ��ư�� ���� ��� ȣ��
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����
			return TRUE;
		}
	}
	return FALSE;
}

// ip�� ��ȿ�� ��Ƽĳ��Ʈ ip���� Ȯ���ϴ� �Լ�
BOOL check_ip(char* ip_addr, int len) {
	char tempString[40];
	char *ptr;
	int t = 0;
	strncpy(tempString, ip_addr, len+1);

	ptr = strtok(tempString, ".");
	t = atoi(ptr);
	if (t > 239 || t < 224) { // ù���� IP�ּҰ� Multicast �ּ�(Class D)�� �ƴ� ��� ����
		return FALSE;
	}

	for (int i = 0; i < 2; i++) {
		strtok(NULL, ".");
	}

	ptr = strtok(NULL, ".");
	t = atoi(ptr);
	if (t == 0 || t == 255) // IP�ּ��� ���������� 0 �Ǵ� 255�̸� �ּҰ� ����
		return FALSE;

	return TRUE;
}

// �Էµ� port ��ȣ�� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_port(char* port) {
	int t;
	for (int i = 0; i < strlen(port); i++) {
		if (port[i] > 0x39 || port[i] < 0x30) // ��Ʈ��ȣ�� ���ڰ��� �ƴϸ� ����
			return FALSE;
	}

	t = atoi(port);

	if (t > 65535 || t < 1024) // ��Ʈ��ȣ�� ��Ʈ ���� ���� ���̰ų� 1024���� ������ ����
		return FALSE;
	return TRUE;
}

// �Էµ� ��ȭ���� ��ȿ���� Ȯ���ϴ� �Լ�
BOOL check_nick(char* nickname) {
	if (strlen(nickname) == 0) // ��ȭ�� ���̰� 0�̸� ����
		return FALSE;
	if (nickname[0] == '\n' || nickname[0] == '\r') // ��ȭ�� ù ���ڰ� ���๮�ڸ� ����
		return FALSE;
	return TRUE;
}	
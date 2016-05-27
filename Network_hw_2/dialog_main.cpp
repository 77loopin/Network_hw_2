#include "resource.h"
#include "main.h"
#include "socket_multicast.h"


// Main Dialog Box
BOOL CALLBACK main_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static char buf[SOCKETBUFSIZE + 1]; // ����ڰ� �Է��� �޽����� �����ϴ� ����
	char myNickname[NICKBUFSIZE + 1]; // ������� ��ȭ���� �����ϴ� �ӽ� ����
	int myConnect; // ������� ���� ���¸� �����ϴ� �ӽ� ����

	switch (uMsg) {
	case WM_INITDIALOG: // DialogBox�� ó�� ���� ���� �� �ʱ�ȭ
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1�� �ڵ鰪�� �������ش�.
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2); // IDC_EDIT2�� �ڵ鰪�� �������ش�.
		hMenu = GetDlgItem(hDlg, IDR_MENU1); // IDR_MENU1�� �ڵ鰪�� �������ش�.
		SendMessage(hEdit1, EM_SETLIMITTEXT, SOCKETBUFSIZE, 0); // hEdit1�� �ִ� �Է� ũ�⸦ �����Ѵ�.
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, setup_DlgProc); // ���� ������ ���� Setup DialogBox�� �����Ѵ�.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // OK��ư�� �����ų� Enter key�� ���� ��� ȣ��
			GetDlgItemText(hDlg, IDC_EDIT1, buf, SOCKETBUFSIZE + 1); // IDC_EDIT1�� ����� �����͸� buf�� �����Ѵ�.
			// hDlg : ��ȭ���� �ڵ�, IDC_EDIT1 : ��Ʈ��ID, buf : ������ ���� �ּ�, ���� ũ��
			if (strlen(buf) == 0) { // �Է� ������ ������ Focus�� ������ �� �ٷ� ����
				SetFocus(hEdit1);
				return TRUE;
			}

			// setting ����ü�� sendMsg ����ü�� ����
			// Critical Section ����
			EnterCriticalSection(&cs);
			strncpy(myNickname, setting.nickname, NICKBUFSIZE+1);
			strncpy(sendMsg.nickname, myNickname, NICKBUFSIZE + 1);
			strncpy(sendMsg.data, buf, NICKBUFSIZE+1);
			sendMsg.messageFlag = 1;
			myConnect = setting.connectFlag;
			LeaveCriticalSection(&cs);
			// setting�� nickname�� connectFlg�� �̿��Ͽ� sendMsg�� ������

			
			// �޽����� ������ ���� Sender�Լ� ȣ�� (Thread ����)
			hSender = CreateThread(NULL, 0, MulChat_Msg_Sender, NULL, 0, NULL);
			while (hSender == NULL) hSender = CreateThread(NULL, 0, MulChat_Msg_Sender, NULL, 0, NULL);
			CloseHandle(hSender);

			// �޽����� ���� �Ŀ� �Է� ������ ��Ʈ���� ����.
			SendMessage(hEdit1, EM_SETSEL, 0, -1); // hEdit1�� �ִ� ���ڿ��� ��� ���� ���·� �����.
			SendMessage(hEdit1, EM_REPLACESEL, FALSE, (LPARAM)"");
			SetFocus(hEdit1); // �Է� ������ ��Ʈ�ѿ� �ٽ� Focus��Ų��.
			return TRUE;
			
		case IDCANCEL: // ��� ��ư�� ���� ���
			EndDialog(hDlg, IDCANCEL); // ��ȭ���� ����, �� ���α׷� ����
			return TRUE;
			
		case ID_40001: // �޴��ٿ��� "���� ����"�� ������ ���
			// ���� ������ ���� setup DialogBox�� ȣ��
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, setup_DlgProc);
			return TRUE;
			
		case ID_40002: // �޴��ٿ��� "��ȭ�� ����"�� ������ ���
			// ��ȭ�� ������ ���� nickname DialogBox�� ȣ��
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hDlg, nickname_DlgProc);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}



// �޽��� ����� ���� ������ ��Ʈ�� �Լ�
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[SOCKETBUFSIZE + NICKBUFSIZE + 10];
	vsprintf(cbuf, fmt, arg); // �Էµ� ���ڿ��� �ϼ��ؼ� cbuf�� �����Ѵ�.
	int nLength = GetWindowTextLength(hEdit2); // �ش� ���ڿ��� ���̸� ��ȯ�Ѵ�.
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength); // ���� hEdit2�� ���� �ڷ� �����͸� �̵� �� ��
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf); // ���� ��ġ�� cbuf ������ ����Ѵ�.
	va_end(arg);
}
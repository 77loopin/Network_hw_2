#include "resource.h"
#include "main.h"
#include "socket_multicast.h"


// Main Dialog Box
BOOL CALLBACK main_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static char buf[SOCKETBUFSIZE + 1]; // 사용자가 입력한 메시지를 저장하는 변수
	char myNickname[NICKBUFSIZE + 1]; // 사용자의 대화명을 저장하는 임시 변수
	int myConnect; // 사용자의 접속 상태를 저장하는 임시 변수

	switch (uMsg) {
	case WM_INITDIALOG: // DialogBox가 처음 실행 됐을 때 초기화
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1의 핸들값을 리턴해준다.
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2); // IDC_EDIT2의 핸들값을 리턴해준다.
		hMenu = GetDlgItem(hDlg, IDR_MENU1); // IDR_MENU1의 핸들값을 리턴해준다.
		SendMessage(hEdit1, EM_SETLIMITTEXT, SOCKETBUFSIZE, 0); // hEdit1의 최대 입력 크기를 설정한다.
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, setup_DlgProc); // 최초 설정을 위해 Setup DialogBox를 실행한다.
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: // OK버튼을 누르거나 Enter key를 누른 경우 호출
			GetDlgItemText(hDlg, IDC_EDIT1, buf, SOCKETBUFSIZE + 1); // IDC_EDIT1에 저장된 데이터를 buf에 저장한다.
			// hDlg : 대화상자 핸들, IDC_EDIT1 : 컨트롤ID, buf : 버퍼의 시작 주소, 버퍼 크기
			if (strlen(buf) == 0) { // 입력 내용이 없으면 Focus를 수정한 후 바로 종료
				SetFocus(hEdit1);
				return TRUE;
			}

			// setting 구조체와 sendMsg 구조체에 접근
			// Critical Section 접근
			EnterCriticalSection(&cs);
			strncpy(myNickname, setting.nickname, NICKBUFSIZE+1);
			strncpy(sendMsg.nickname, myNickname, NICKBUFSIZE + 1);
			strncpy(sendMsg.data, buf, NICKBUFSIZE+1);
			sendMsg.messageFlag = 1;
			myConnect = setting.connectFlag;
			LeaveCriticalSection(&cs);
			// setting의 nickname과 connectFlg를 이용하여 sendMsg를 설정함

			
			// 메시지를 보내기 위해 Sender함수 호출 (Thread 생성)
			hSender = CreateThread(NULL, 0, MulChat_Msg_Sender, NULL, 0, NULL);
			while (hSender == NULL) hSender = CreateThread(NULL, 0, MulChat_Msg_Sender, NULL, 0, NULL);
			CloseHandle(hSender);

			// 메시지를 보낸 후에 입력 에디터 컨트롤을 비운다.
			SendMessage(hEdit1, EM_SETSEL, 0, -1); // hEdit1에 있는 문자열을 모두 선택 상태로 만든다.
			SendMessage(hEdit1, EM_REPLACESEL, FALSE, (LPARAM)"");
			SetFocus(hEdit1); // 입력 에디터 컨트롤에 다시 Focus시킨다.
			return TRUE;
			
		case IDCANCEL: // 취소 버튼을 누른 경우
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료, 즉 프로그램 종료
			return TRUE;
			
		case ID_40001: // 메뉴바에서 "설정 변경"을 선택한 경우
			// 설정 변경을 위해 setup DialogBox를 호출
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, setup_DlgProc);
			return TRUE;
			
		case ID_40002: // 메뉴바에서 "대화명 변경"을 선택한 경우
			// 대화명 변경을 위해 nickname DialogBox를 호출
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hDlg, nickname_DlgProc);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}



// 메시지 출력을 위한 에디터 컨트롤 함수
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[SOCKETBUFSIZE + NICKBUFSIZE + 10];
	vsprintf(cbuf, fmt, arg); // 입력된 문자열을 완성해서 cbuf에 저장한다.
	int nLength = GetWindowTextLength(hEdit2); // 해당 문자열의 길이를 반환한다.
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength); // 현재 hEdit2의 길이 뒤로 포인터를 이동 한 후
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf); // 현재 위치에 cbuf 내용을 출력한다.
	va_end(arg);
}
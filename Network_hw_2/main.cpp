#include <Windows.h>
#include <stdio.h>


#define BUFSIZE 150

// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...);

HINSTANCE hInst;
HWND hEdit;
HWND hEdit1, hEdit2; // 편집 컨트롤

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	hInst = hInstance;

	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "MyWndClass";
	if (!RegisterClass(&wndclass)) return 1;

	HWND hWnd = CreateWindow("MyWndClass", "Title", WS_OVERLAPPEDWINDOW, 0, 0, 600, 600, NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) return 1;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
	//DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	//return 0;
}
/*
// 대화상자 프로시저
BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static char buf[BUFSIZE + 1];


	switch (uMsg) {
	case WM_INITDIALOG: // 프로그램을 처음 실행 했을 때 초기화
		hEdit1 = GetDlgItem(hDlg, IDC_EDIT1); // IDC_EDIT1의 핸들값을 리턴해준다.
		hEdit2 = GetDlgItem(hDlg, IDC_EDIT2); // IDC_EDIT2의 핸들값을 리턴해준다.
		SendMessage(hEdit1, EM_SETLIMITTEXT, BUFSIZE, 0); // hEdit1의 최대 입력 크기를 설정한다.
														  // recevier 스레드 돌리기
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT1, buf, BUFSIZE + 1); // IDC_EDIT1에 저장된 데이터를 buf에 저장한다.
															   // hDlg : 대화상자 핸들, IDC_EDIT1 : 컨트롤ID, buf : 버퍼의 시작 주소, 버퍼 크기
			if (strlen(buf) == 0) {
				SetFocus(hEdit1);
				return TRUE;
			}
			DisplayText("%s\r\n", buf);
			// sender 스레드 돌리기
			SendMessage(hEdit1, EM_SETSEL, 0, -1); // hEdit1에 있는 문자열을 모두 선택 상태로 만든다.
			SendMessage(hEdit1, EM_REPLACESEL, FALSE, (LPARAM)"");
			SetFocus(hEdit1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL); // 대화상자 종료
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}
*/

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE:
		hEdit = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | WS_DLGFRAME
			, 10, 10, 560, 400, hWnd, (HMENU)100, hInst, NULL);
		//hEdit2 = CreateWindow("edit", NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE
		//	,10 , 30, 100, 300, hWnd, (HMENU)100, hInst, NULL);
		DisplayText("안녕하세요.\r\n");
		return 0;
		//case WM_SIZE:
		//	MoveWindow(hEdit, 50, 50, LOWORD(lParam), HIWORD(lParam), TRUE);
		//	return 0;
	case WM_SETFOCUS:
		SetFocus(hEdit);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}




// 편집 컨트롤 출력 함수
void DisplayText(char *fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char cbuf[BUFSIZE + 256];
	vsprintf(cbuf, fmt, arg); // 입력된 문자열을 완성해서 cbuf에 저장한다.
	int nLength = GetWindowTextLength(hEdit2); // 해당 문자열의 길이를 반환한다.
	SendMessage(hEdit2, EM_SETSEL, nLength, nLength); // 
	SendMessage(hEdit2, EM_REPLACESEL, FALSE, (LPARAM)cbuf);
	va_end(arg);
}
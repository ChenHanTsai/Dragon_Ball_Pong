
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <commdlg.h>
#include <basetsd.h>
#include <objbase.h>



#include <Windows.h> // Include the Windows API (Win32 Application Programming Interface)
#include <tchar.h> // Include UNICODE support
#include <sstream> // Include ostringstream support
using std::wostringstream;

#include <commdlg.h> // Include the Common Dialogs.
#include <process.h> // Include Multi-Threading functions.
#include "resource.h" // Include the resource script IDs.


#include "Game.h" // Include the Game Class.

bool cpuControl_1p = false;

bool cpuControl_2p = true;

GAMESTATE m_state;

Game pong; // The Game Instance (only global temporarily).

//detect whether go back to the replay
bool newFile = false;

//detect the pause button
bool pause = false;

//string table
#define MAX_LOADSTRING 100

//string Global Variables:
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szWin[MAX_LOADSTRING];					// The title bar text
TCHAR szLose[MAX_LOADSTRING];			// the main window class name
TCHAR szPause[MAX_LOADSTRING];					// The title bar text
TCHAR szOpen[MAX_LOADSTRING];					// The title bar text

// Foward Declarations:
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hWnd);
void OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

bool toogleFull = false;
// Function that toggles a window between fullscreen and windowed mode:
void ToggleFullscreenMode(HWND hWnd, BOOL bFullScreen);


void randomColor(D2D1_COLOR_F& _gameBoardColor, D2D1_COLOR_F& _fill_start, D2D1_COLOR_F& _fill_stop);

// Entry point function for the game:
unsigned int WINAPI GameMain(void* lpvArgList);
void CPUCheck(HWND);

//Default game board line color
D2D1_COLOR_F gameBoardColor = D2D1::ColorF(D2D1::ColorF::White);
//Default O FILL COLOR
D2D1_COLOR_F O_FILL_Color_Start = D2D1::ColorF(D2D1::ColorF::Red);
D2D1_COLOR_F O_FILL_Color_Stop = D2D1::ColorF(D2D1::ColorF::Yellow);

//menu
// Global Variables:
HINSTANCE hInst;								// current instance
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	HowToPlay(HWND, UINT, WPARAM, LPARAM);
// The Entry Point function
int WINAPI _tWinMain( HINSTANCE hInstance,
					 HINSTANCE hPrevInstance, // Always NULL
					 LPTSTR	lpCmdLine,
					 int		iCmdShow )
{
	// Initialize the COM Library.
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	HACCEL hAccelTable;

	

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_TICTACTOE, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_WIN, szWin, MAX_LOADSTRING);
	LoadString(hInstance, IDS_Pause, szPause, MAX_LOADSTRING);
	LoadString(hInstance, IDS_Title, szOpen, MAX_LOADSTRING);
	// Step 1: Initialize the Window Class.
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(wcex);
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wcex.hInstance = hInstance;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc; // Function to send Windows Message to for processing.
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_REALTIME);
	wcex.lpszClassName = _T("REALTIMEWND"); // Name to identify this Window Class.
	RegisterClassEx(&wcex);


	// Step 2: Create the Main Window.
	HWND hWnd = CreateWindow(wcex.lpszClassName,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1200, 900,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL);
	if (! hWnd)
	{
		//DWORD err = GetLastError();
		MessageBox(HWND_DESKTOP, _T("ERROR: Failed to create main window!"),
			_T("Program Name"), MB_OK | MB_ICONERROR);
		return -1;
	}


	CheckMenuRadioItem(GetMenu(hWnd), ID_1P_CPU, ID_1P_PLAYER, ID_1P_PLAYER, MF_BYCOMMAND);
	CheckMenuRadioItem(GetMenu(hWnd), ID_2P_CPU, ID_2P_PLAYER, ID_2P_CPU, MF_BYCOMMAND);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR2));

	// Step 2.5: Create the Game thread:
	pong.hGameThread = (HANDLE)_beginthreadex(NULL, 0,
		GameMain, (void*)hWnd, 0, NULL);
	#ifdef _DEBUG
		Sleep(1);  // Sleep after thread creation for debugging to work properly.
	#endif


	// Step 3: Show the Main Window.
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	
	// Step 4: Enter the Main Message Loop.
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0)) // Getting WM_QUIT makes GetMessage return FALSE.
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg); // Generate WM_CHAR messages (case sensitive input).
			DispatchMessage(&msg); // Invoke the WindowProc for message processing.
		}
	
	}


	// Destroy the Game Thread:
	CloseHandle(pong.hGameThread);


	// Uninitialize the COM Library.
	CoUninitialize();


	return msg.wParam;
}


// Entry point function for the game:
unsigned int WINAPI GameMain(void* lpvArgList)
{
	// Initialize the COM Library.
	CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);

	// Increase the accuracy/precision of the Windows timer:
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	timeBeginPeriod(tc.wPeriodMin);


	// Get the HWND value out of the ArgList parameter.
	HWND hWnd = (HWND)lpvArgList;

	pong.SetHwnd(hWnd);
	
	m_state = GAME_INIT;
	while (pong.bRunning)
	{
			switch (m_state)
			{
			
			case GAME_INIT:
				newFile = false;
				pause = false;
				pong.Startup();
				m_state = GAME_TITLE;
				break;	
			case GAME_TITLE:
				m_state = pong.title();
				//m_state = GAME_INIT;
				break;
			case GAME_INPUT:	
				if (pause)
				{
					//do nothing, no input here 
					m_state = GAME_DRAW;
				}
				else
					m_state = pong.Input();
				break;
			case GAME_UPDATE:	
				if (newFile)
				{
					m_state = GAME_INIT;
				}
				else if (pause)
				{
					//do nothing, no update here 
				}
				else
				{
					pong.SetCPU(cpuControl_1p, cpuControl_2p);
					m_state = pong.Simulate();
				}
				
				break;
			case GAME_DRAW:
				m_state = pong.Render(pause, szOpen, szPause, gameBoardColor, O_FILL_Color_Start, O_FILL_Color_Stop);
				break;
			case GAME_ENEMY_WIN:
				m_state = pong.gameReplay();
				break; 
			case GAME_PLAYER_WIN:
				m_state = pong.gameReplay();
				break;
			case GAME_END:
				pong.bRunning = false;
				break;
		}
		// 1 Frame == 1 iteration of Input, Simulate, & Render
	}

	pong.Shutdown();


	// Restore the accuracy/precision of the Windows timer:
	timeEndPeriod(tc.wPeriodMin);

	// Uninitialize the COM Library.
	CoUninitialize();

	return 0;
}


// Step 5: Write the Window Procedure
LRESULT CALLBACK WindowProc(HWND hWnd,
							UINT uMsg,
							WPARAM wParam,
							LPARAM lParam)
{
	switch (uMsg)
	{
		// Handle Window Creation:
	case WM_CREATE:
		break;

		// Handle Close Button [X] Pressed:
	case WM_CLOSE:
		// Begin GameMain shutdown.
		pong.bRunning = FALSE;

		// Wait until Game Thread terminates.
		WaitForSingleObject(pong.hGameThread, INFINITE);

		// Begin WinMain shutdown.
		DestroyWindow(hWnd);
		break;

		// Handle Window Destruction:
	case WM_DESTROY:
		PostQuitMessage(0); // Put the WM_QUIT message in the message queue.
		break;


		// Handle Window Rendering:
	case WM_PAINT: //(output)
		OnPaint(hWnd);
		break;

		// Handle Left Mouse Button Clicked:
	case WM_LBUTTONDOWN: //(input)
		OnLButtonDown(hWnd, wParam, lParam);
		break;

		// Handle keyboard input:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		GetKeyboardState(pong.keys);
		DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;

		// Handle window activation:
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_INACTIVE)
			GetKeyboardState(pong.keys);
		else
		{
			ZeroMemory(pong.keys, 256);
			
		}		
		break;

		// Handle Menu Items:
	case ID_SETTING_FULLSCREEN:

		break;
	case WM_ERASEBKGND:
		break;
	
	case WM_COMMAND:
		OnCommand(hWnd, wParam, lParam);
		break;

	default:
		// Allow Windows to perform default processing.
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

void OnPaint(HWND hWnd)
{
	ValidateRect(hWnd, NULL); // VERY IMPORTANT - Clear the Invalid flag
}

void OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	GetClientRect(hWnd, &rc);

	POINT ptMouse;
	ptMouse.x = LOWORD(lParam); // Bits 0-15 contain the X Axis position
	ptMouse.y = HIWORD(lParam); // Bits 16-31 contain the Y Axis position

	InvalidateRect(hWnd, NULL, TRUE);
}

void OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int menuid = LOWORD(wParam);
	switch(menuid)
	{
	case IDM_FILE_NEW:
		newFile = true;
		break;

	case ID_SETTING_FULLSCREEN:
		toogleFull = !toogleFull;
		ToggleFullscreenMode(hWnd, toogleFull);
		break;
	case ID_ABOUT_CREDITS:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, About);
		break;
	case ID_SETTING_PAUSECTRL:
		pause = !pause;
		break;
	case ID_1P_CPU:
	case ID_1P_PLAYER:
		CheckMenuRadioItem(GetMenu(hWnd), ID_1P_CPU, ID_1P_PLAYER, menuid, MF_BYCOMMAND);
		CPUCheck(hWnd);
		break;
	case ID_2P_CPU:
	case ID_2P_PLAYER:
		CheckMenuRadioItem(GetMenu(hWnd), ID_2P_CPU, ID_2P_PLAYER, menuid, MF_BYCOMMAND);
		CPUCheck(hWnd);
		break;
	case ID_ABOUT_HOWTOPLAY:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, HowToPlay);
		break;
	case ID_LIFEBARBOARDCOLOR_RANDOM:
		randomColor(gameBoardColor, O_FILL_Color_Start, O_FILL_Color_Stop);
		break;
	case IDM_FILE_EXIT:
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	}
}

void randomColor(D2D1_COLOR_F& gameBoardColor, D2D1_COLOR_F& O_FILL_Color_Start, D2D1_COLOR_F& O_FILL_Color_Stop)
{
	gameBoardColor.r = ((float)(rand() % 255)) / 255.0f;
	gameBoardColor.g = ((float)(rand() % 255)) / 255.0f;
	gameBoardColor.b = ((float)(rand() % 255)) / 255.0f;

	O_FILL_Color_Start.r = ((float)(rand() % 255)) / 255.0f;
	O_FILL_Color_Start.g = ((float)(rand() % 255)) / 255.0f;
	O_FILL_Color_Start.b = ((float)(rand() % 255)) / 255.0f;

	O_FILL_Color_Stop.r = ((float)(rand() % 255)) / 255.0f;
	O_FILL_Color_Stop.g = ((float)(rand() % 255)) / 255.0f;
	O_FILL_Color_Stop.b = ((float)(rand() % 255)) / 255.0f;
}
// Function that toggles a window between fullscreen and windowed mode:
void ToggleFullscreenMode(HWND hWnd, BOOL bFullScreen)
{
	static WINDOWPLACEMENT wp = { 0 };
	static HMENU hMenu = NULL;

	if (bFullScreen)
	{
		// Remember the window position.
		wp.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(hWnd, &wp);

		// Remove the window's title bar.
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);

		// Put the changes to the window into effect.
		SetWindowPos(hWnd, 0, 0, 0, 0, 0,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

		// Remember the menu, then remove it.
		hMenu = GetMenu(hWnd);
		SetMenu(hWnd, NULL);

		// Switch to the requested display mode.
		//SetDisplayMode(iWidth, iHeight, iBpp, iRefreshRate);

		// Position the window to cover the entire screen.
		SetWindowPos(hWnd, HWND_TOPMOST,
			0, 0,
			GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			SWP_SHOWWINDOW);

		// Remove the cursor.
		ShowCursor(FALSE);
	}
	else
	{
		// Restore the window's title bar.
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

		// Put the changes to the window into effect.
		SetWindowPos(hWnd, 0, 0, 0, 0, 0,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

		// Restore the window's menu.
		SetMenu(hWnd, hMenu);

		// Restore the display mode.
		//SetDisplayMode(0, 0, 0, 0);

		// Restore the window's original position.
		SetWindowPlacement(hWnd, &wp);

		// Restore the cursor.
		ShowCursor(TRUE);
	}
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Message handler for about box.
INT_PTR CALLBACK HowToPlay(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void CPUCheck(HWND hWnd)
{
	HMENU hMenu = GetMenu(hWnd);

	int menuState = GetMenuState(hMenu, ID_1P_CPU, MF_BYCOMMAND);

	if (menuState & MF_CHECKED)
		cpuControl_1p = true;
	else
		cpuControl_1p = false;

	 menuState = GetMenuState(hMenu, ID_2P_CPU, MF_BYCOMMAND);

	if (menuState & MF_CHECKED)
		cpuControl_2p = true;
	else
		cpuControl_2p = false;

}
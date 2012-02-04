
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include <string>
#include <list>

#include "ServerConnection.h"

#include "../RconLibrary/RconPacket.h"

HINSTANCE hInstance;

enum { HistoryBoxId = 100 };
enum { EditBoxId = 101 };

enum { EditBoxHeight = 24 };

enum { InitialWindowWidth = 600 };
enum { InitialWindowHeight = 400 };

HWND editBoxHwnd;
WNDPROC editBoxOriginalWndProc;

HWND historyBoxHwnd;

std::string historyBoxContent;

typedef std::list<std::string> CommandHistory;

CommandHistory commandHistory;
CommandHistory::const_iterator currentCommandHistoryPos;

ServerConnectionThread* serverConnection = nullptr;

void tokenize(const char* string, Words& words)
{

	enum State
	{
		ScanningForWordBeginning,
		ScanningForWordEnd,
	};

	State state = ScanningForWordBeginning;

	const char* wordStart = string;
	const char* wordEnd = string;

	while (true)
	{
		char ch = *wordEnd;

		switch (state)
		{
		case ScanningForWordBeginning:
			{
				if (ch == '\0')
				{
					return;
				}
				else if (ch != ' ' && ch != '\t')
				{
					wordStart = wordEnd;
					wordEnd++;
					state = ScanningForWordEnd;
				}
			}
			break;

		case ScanningForWordEnd:
			{
				if (ch == '\0')
				{
					words.push_back(std::string(wordStart));
					return;
				}
				if (ch != ' ' && ch != '\t')
				{
					wordEnd++;
				}
				else
				{
					words.push_back(std::string(wordStart, wordEnd - wordStart));
					wordEnd++;
					state = ScanningForWordBeginning;
				}
			}
			break;
		}
	}
}

LRESULT CALLBACK EditBoxWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		{
			if (LOWORD(wParam) == VK_RETURN)
			{
				if (serverConnection)
				{
					static char buf[50000];
					SendMessage(editBoxHwnd, WM_GETTEXT, sizeof buf, reinterpret_cast<LPARAM>(buf));
					buf[(sizeof buf) - 1] = '\0';

					commandHistory.push_back(buf);
					currentCommandHistoryPos = commandHistory.end();

					Words words;
					tokenize(buf, words);

					serverConnection->sendRequest(words);

					SendMessage(editBoxHwnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(""));
				}

				return TRUE;
			}
			else if (LOWORD(wParam) == VK_UP)
			{
				if (currentCommandHistoryPos != commandHistory.begin())
				{
					currentCommandHistoryPos--;
					const char* command = currentCommandHistoryPos->c_str();
					SendMessage(editBoxHwnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(command));
					size_t commandLength = strlen(command);
					SendMessage(editBoxHwnd, EM_SETSEL, commandLength, commandLength);
				}
				return TRUE;
			}
			else if (LOWORD(wParam) == VK_DOWN)
			{
				if (currentCommandHistoryPos != commandHistory.end())
				{
					currentCommandHistoryPos++;
					const char* command = (currentCommandHistoryPos != commandHistory.end()) ? currentCommandHistoryPos->c_str() : "";
					SendMessage(editBoxHwnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(command));
					size_t commandLength = strlen(command);
					SendMessage(editBoxHwnd, EM_SETSEL, commandLength, commandLength);
				}
				return TRUE;
			}
		}
		break;
	}

	return CallWindowProc(editBoxOriginalWndProc, hWnd, message, wParam, lParam);
}

void calculateWindowElementSizes(int windowWidth, int windowHeight, RECT& historyBox, RECT& editBox)
{
	historyBox.left = 0;
	historyBox.top = 0;
	historyBox.right = windowWidth;
	historyBox.bottom = windowHeight - EditBoxHeight;
	editBox.left = 0;
	editBox.top = windowHeight - EditBoxHeight;
	editBox.right = windowWidth;
	editBox.bottom = windowHeight;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		{
			RECT historyBoxRect, editBoxRect;

			calculateWindowElementSizes(InitialWindowWidth, InitialWindowHeight, historyBoxRect, editBoxRect);

			historyBoxHwnd = CreateWindow(
				TEXT("EDIT"),                              // The class name required is edit
				TEXT(""),                                 // Default text.
				WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_READONLY, // the styles
				historyBoxRect.left, historyBoxRect.top,                                      // the left and top co-ordinates
				historyBoxRect.right - historyBoxRect.left,
				historyBoxRect.bottom - historyBoxRect.top,        // width and height
				hWnd,                                     // parent window handle
				(HMENU)HistoryBoxId,                         // the ID of your combobox
				hInstance,                                // the instance of your application
				NULL
			);                                   // extra bits you dont really need

			editBoxHwnd = CreateWindow(
				TEXT("EDIT"),                              // The class name required is edit
				TEXT(""),                                 // Default text.
				WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, // the styles
				editBoxRect.left, editBoxRect.top,                                      // the left and top co-ordinates
				editBoxRect.right - editBoxRect.left,
				editBoxRect.bottom - editBoxRect.top,        // width and height
				hWnd,                                     // parent window handle
				(HMENU)EditBoxId,                         // the ID of your combobox
				hInstance,                                // the instance of your application
				NULL
			);                                   // extra bits you dont really need

			editBoxOriginalWndProc = (WNDPROC) GetWindowLong(editBoxHwnd, GWL_WNDPROC);
			SetWindowLong(editBoxHwnd, GWL_WNDPROC, (long) EditBoxWndProc);

		}
		break;

	case WM_DESTROY:
		{
			PostQuitMessage(0);
		}
		break;

	case WM_SETFOCUS:
		{
			SetFocus(editBoxHwnd);
			return 0;
		}
		break;

	case WM_SIZE: 
		{
			RECT historyBoxRect, editBoxRect;
			calculateWindowElementSizes(LOWORD(lParam), HIWORD(lParam), historyBoxRect, editBoxRect);

			MoveWindow(editBoxHwnd, editBoxRect.left, editBoxRect.top,
				editBoxRect.right - editBoxRect.left, editBoxRect.bottom - editBoxRect.top, TRUE);

			MoveWindow(historyBoxHwnd, historyBoxRect.left, historyBoxRect.top,
				historyBoxRect.right - historyBoxRect.left, historyBoxRect.bottom - historyBoxRect.top, TRUE);

			return 0;
		}
		break;

	case WM_USER:
		{
			const char* message = reinterpret_cast<const char*>(lParam);
			historyBoxContent += message;
			historyBoxContent += "\r\n";

			SendMessage(historyBoxHwnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(historyBoxContent.c_str()));
		}
		break;

	default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
	currentCommandHistoryPos = commandHistory.end();

	static TCHAR szWindowClass[] = _T("win32app");
	static TCHAR szTitle[] = _T("RCON Console");

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("Call to RegisterClassEx failed!"), _T("Error"), NULL);

		return 1;
	}

	::hInstance = hInstance; // Store instance handle in our global variable

	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	RECT windowRect = { 0, 0, InitialWindowWidth, InitialWindowHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd)
	{
		MessageBox(NULL, _T("Call to CreateWindow failed!"), _T("Error"), NULL);

		return 1;
	}

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	serverConnection = new ServerConnectionThread(hWnd);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	serverConnection->requestQuit();

	delete serverConnection;

	return (int) msg.wParam;
}


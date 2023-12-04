#include "QRGrabber.h"

#include <ReadBarcode.h>

#define NOMINMAX
#include <Windows.h>

struct MonitorRects
{
    std::vector<RECT>   rcMonitors;
    RECT                rcCombined;

    static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
    {
        MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
        pThis->rcMonitors.push_back(*lprcMonitor);
        UnionRect(&pThis->rcCombined, &pThis->rcCombined, lprcMonitor);
        return TRUE;
    }

    MonitorRects()
    {
        SetRectEmpty(&rcCombined);
        EnumDisplayMonitors(0, 0, MonitorEnum, (LPARAM)this);
    }
};

std::vector<std::string>  ExtractFromScreen() {
    std::vector<std::string> res;
    
    // Get the dimensions of the screen
    MonitorRects screens;
    int screenWidth = screens.rcCombined.right - screens.rcCombined.left;
    int screenHeight = screens.rcCombined.bottom - screens.rcCombined.top;

    if ((screenHeight<=0) || (screenWidth<=0)) return res;    
        
    // Get a handle to the entire screen
    HDC hdcScreen = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);//GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    // Create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);

    // Select the bitmap into the compatible device context
    HGDIOBJ hOrgBMP=SelectObject(hdcMem, hBitmap);

    // Copy the screen to the compatible device context
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, screens.rcCombined.left, screens.rcCombined.top, SRCCOPY);

    BITMAPINFO bi;
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = screenWidth;
    bi.bmiHeader.biHeight = -screenHeight; // otherwise image is upside down
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0;
    bi.bmiHeader.biXPelsPerMeter = 0;
    bi.bmiHeader.biYPelsPerMeter = 0;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;

    GetDIBits(hdcScreen, hBitmap, 0, screenHeight, NULL, &bi, DIB_RGB_COLORS);
    if (bi.bmiHeader.biSizeImage) {
        BYTE* lpBits = new BYTE[bi.bmiHeader.biSizeImage];
        GetDIBits(hdcScreen, hBitmap, 0, screenHeight, lpBits, &bi, DIB_RGB_COLORS);

#if 0
        FILE* filePtr;
        fopen_s(&filePtr, "screenshot.bmp", "wb");
        BITMAPFILEHEADER bmfHeader;
        bmfHeader.bfType = 0x4D42; // 'BM'
        bmfHeader.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + screenWidth * screenHeight * 3);
        bmfHeader.bfReserved1 = 0;
        bmfHeader.bfReserved2 = 0;
        bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

        fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
        fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, filePtr);

        fwrite(lpBits, 1, screenWidth * screenHeight * 3, filePtr);

        fclose(filePtr);
#endif

        try {            
            ZXing::DecodeHints hints;
            hints.setFormats(ZXing::BarcodeFormat::QRCode);
            hints.setMaxNumberOfSymbols(5);
            hints.setIsPure(false);
            hints.setTryRotate(false);
            hints.setTryHarder(false);
            hints.setTryInvert(false);
            auto r = ZXing::ReadBarcodes(ZXing::ImageView(lpBits, screenWidth, screenHeight, ZXing::ImageFormat::RGB), hints);                                  

            for (const auto& i : r)
                if (i.text().find("otpauth") != std::string::npos)
                    res.push_back(std::move(i.text()));
        }
        catch (...) {
        }
        delete[] lpBits;
    }

    // Cleanup
    SelectObject(hdcMem, hOrgBMP);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return res;
}

#ifdef _DEBUG
#include <iostream>
int __stdcall goCallback2(const char* c, int) {
    std::cout << " result: " << c << std::endl;
    return 0;
}
typedef int (__stdcall * goCallback)(const char*, int);
extern "C" {
    __declspec(dllimport) int __cdecl monitor(char* argsBuffer, uint32_t bufferSize, goCallback callback);
}

int main()
{

    char a[] = "1";
    char b[] = "2";
    monitor(a, 1, &goCallback2);

    while (true) {
        Sleep(10 * 1000);
        monitor(b, 1, &goCallback2);
    }
 
    return 0;
}
#endif

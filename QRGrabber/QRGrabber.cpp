#include "QRGrabber.h"
#ifdef _DEBUG
#include <iostream>
#endif
#define NOMINMAX
#include <Windows.h>

ZXing::Results ExtractFromScreen(){
    ZXing::Results res;
    
    // Get the dimensions of the screen
    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);  // XXXX test multiscreen
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (!screenHeight || !screenWidth) return res;

    // Get a handle to the entire screen
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    // Create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);

    // Select the bitmap into the compatible device context
    SelectObject(hdcMem, hBitmap);

    // Copy the screen to the compatible device context
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenWidth;
    bi.biHeight = screenHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    GetDIBits(hdcScreen, hBitmap, 0, screenHeight, NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    BYTE* lpBits = new BYTE[screenWidth * screenHeight * 4];
    GetDIBits(hdcScreen, hBitmap, 0, screenHeight, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    try {
        ZXing::ImageView image{ lpBits, screenWidth, screenHeight, ZXing::ImageFormat::RGBX };

        res = ZXing::ReadBarcodes(image);
    }catch(...){        
    }

#ifdef _DEBUG
    for (auto& r : res)
        std::cout << "Live Decoded text: " << r.text() << std::endl;
#endif
    delete[] lpBits;    

    // Cleanup
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    return res;
}

#ifdef _DEBUG
int goCallback2(const char* c, int) {
    std::cout << " result: " << c << std::endl;
    return 0;
}
typedef int (*goCallback)(const char*, int);
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

#include "QRGrabber.h"

#include <ReadBarcode.h>

#define NOMINMAX
#include <Windows.h>

std::vector<std::string>  ExtractFromScreen(){
    std::vector<std::string> res;
    
    // Get the dimensions of the screen
    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN); 
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (!screenHeight || !screenWidth) return res;
    if ((screenWidth > 65535) || (screenHeight > 65535)) return res;
        
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
    bi.biHeight = -screenHeight; // otherwise image is upside downf
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    GetDIBits(hdcScreen, hBitmap, 0, screenHeight, NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    BYTE* lpBits = new BYTE[screenWidth * screenHeight * 3];
    GetDIBits(hdcScreen, hBitmap, 0, screenHeight, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    try {
        ZXing::ImageView image{ lpBits, screenWidth, screenHeight, ZXing::ImageFormat::RGB };

        auto hints = ZXing::DecodeHints();
        hints.setFormats(ZXing::BarcodeFormat::QRCode);
        hints.setMaxNumberOfSymbols(3);     
        auto r = ZXing::ReadBarcodes(image, hints);
        for (auto& i : r)
            if(i.text().find("otpauth") != std::string::npos)
                res.push_back(i.text());
    }catch(...){      
    }
    delete[] lpBits;    

    // Cleanup
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

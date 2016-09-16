/* *************************************************************************************************
* LabelInspection  - Created: 08/23/2016  3:25:59 PM
* Creator Robert K Young - rkyoung@sonic.net
* ChangeLog:
* 0.0.1 - 8/23/2016 3:32:33 PM - Initial Version
* 0.0.2 - 8/24/2016 4:22:15 PM - Added capture file name based on date/time
* 0.1.0 - 8/15/2016 3:38:58 PM - First beta version with basic functionality
* 0.2.0 - 8/25/2016 1:36:01 PM - Incorporated OpenCV
* 0.3.0 - 9/01/2016 5:21:43 PM - Began implementing MediaFoundation calls to control camera.
* 0.4.0 - 9/13/2016 9:11:15 AM - Implementation of MediaFoundation failed, moving on to processing image
//**************************************************************************************************/

#include "stdafx.h"

#define POINT_GREY_CAMERA 1

#ifdef POINT_GREY_CAMERA
#define CANERA_WIDTH 1288 
#define CANERA_HEIGHT 964 
#else
#define CANERA_WIDTH 1280 
#define CANERA_HEIGHT 720 
#endif


using namespace cv;
using namespace std;

namespace std { 
#include <cstdlib> 
};

#define BLUE    0x0001
#define GREEN   0x0002
#define RED     0x0004
#define GRAY    0x0007	

//Remember to Link to vfw32 Library, gdi32 Library  

#define BLUE    0x0001
#define GREEN   0x0002
#define RED     0x0004
#define GRAY    0x0007	

#define MAX_LOADSTRING 100
#define MAX_CAMERAS 4


// Contributing Source Used: http://www.dreamincode.net/forums/topic/193519-win32-webcam-program/

HWND hWindow;
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
bool CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
LPCTSTR szAppName = L"FrameGrab";
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
// void drawImage(HDC screen);
void drawImage(HDC screen, HDC *imageDC);
void Get_DeviceInfo();
int enum_devices();
// void process_filter(IBaseFilter *pBaseFilter);
void process_filter(IBaseFilter *pBaseFilter, int iIndex);
HRESULT CamCaps(IBaseFilter *pBaseFilter, int iIndex);
// HRESULT CamCaps(IBaseFilter *pBaseFilter);
void _FreeMediaType(AM_MEDIA_TYPE& mt);
static void setcolor(unsigned int color);
// void loadImage(const char* pathname);
void loadImage(const char* pathname, HDC *imageDC, HBITMAP *imageBmp);
void cleanUpImage(HDC *imageDC, HBITMAP *imageBmp);
BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP *phBitmap, HPALETTE *phPalette);

HWND mli_CameraHwnd;
int mli_WindowX_Dim;
int mli_WindowY_Dim;
bool mil_bCameraConnected = false;
wchar_t* mfg_CameraNames[MAX_CAMERAS];
bool mil_bDrawImage = false;
wchar_t mil_DbgMesg[MAX_PATH];
// HDC hdc;
HDC hdcMem;
HGDIOBJ mil_hgdiOld;
PAINTSTRUCT ps;
HBITMAP hbm;
RECT rc;
HDC         WorkimageDC;        // the DC to hold our image
HBITMAP     image0Bmp;       // the actual bitmap which contains the image (will be used as display and to draw on)
HBITMAP     image1Bmp;       // the actual bitmap which contains the image (will be what we restore the display with to erase anything or everything)
HPALETTE      hPalette;
HBITMAP     imageBmpOld;    // the DC's old bitmap (for cleanup)
HBITMAP     OriginalimageBmp;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                li_RegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

	// int result;
	// Get_DeviceInfo();
	int result = enum_devices();

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LABELINSPECTION, szWindowClass, MAX_LOADSTRING);
	li_RegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABELINSPECTION));


    MSG msg;

    // Main message loop:
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWindow, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM li_RegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABELINSPECTION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LABELINSPECTION);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // Get the desktop dims and take a little off all four sides
   mli_WindowX_Dim = (GetSystemMetrics(SM_CXSCREEN)); // -(GetSystemMetrics(SM_CXSCREEN) >> 5));
   mli_WindowY_Dim = (GetSystemMetrics(SM_CYSCREEN)); // -(GetSystemMetrics(SM_CYSCREEN) >> 5));

   if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Creating MainWindow: %04d x %04d\n", mli_WindowX_Dim, mli_WindowY_Dim) > 0))
	   OutputDebugString(mil_DbgMesg);

   // hWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, hInstance, nullptr);
   hWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, mli_WindowX_Dim, mli_WindowY_Dim, nullptr, nullptr, hInstance, nullptr);

   if (!hWindow)
   {
      return FALSE;
   }

   ShowWindow(hWindow, nCmdShow);
   UpdateWindow(hWindow);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	//some buttons
	HWND hButtStartCam;
	HWND hButtStopCam;
	HWND hButtGrabFrame;

    switch (message)
    {
	case WM_CTLCOLORSTATIC:
		SetBkMode(hdcMem, TRANSPARENT);
		return (LRESULT)CreateSolidBrush(0xFFFFFF);

	case WM_CREATE:
	{
		hButtStartCam = CreateWindowEx(0, L"BUTTON", L"Start Camera", WS_CHILD | WS_VISIBLE, 0, 0, 300, 60, hWnd, (HMENU)1, hInstance, 0);
		hButtStopCam = CreateWindowEx(0, L"BUTTON", L"Stop Camera", WS_CHILD | WS_VISIBLE, 0, 75, 300, 60, hWnd, (HMENU)2, hInstance, 0);
		hButtGrabFrame = CreateWindowEx(0, L"BUTTON", L"Create Ref Image", WS_CHILD | WS_VISIBLE, 0, 150, 300, 60, hWnd, (HMENU)3, hInstance, 0);

		DWORD dwStyle = WS_CHILD;

		if((mli_WindowX_Dim - 300) < CANERA_WIDTH )
			dwStyle |= WS_HSCROLL;

		if ( mli_WindowY_Dim  < CANERA_HEIGHT )
			dwStyle |= WS_VSCROLL;

		mli_CameraHwnd = capCreateCaptureWindow(L"camera window", dwStyle, 301, 0, CANERA_WIDTH, CANERA_HEIGHT, hWnd, 0);

		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Creating Camera Window: %04d x %04d\n", CANERA_WIDTH, CANERA_HEIGHT) > 0))
			OutputDebugString(mil_DbgMesg);

		SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
		break;
	}


    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;

				// *******************************************************
				case 1:
				{
					// SendMessage(camhwnd, WM_CAP_DLG_VIDEOSOURCE, 0, 0);
					// SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
					// SendMessage(mfg_CameraHwnd, WM_CAP_DRIVER_CONNECT, mfg_CurrentCamera, 0);
					SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
					SendMessage(mli_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
					SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
					SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
					ShowWindow(mli_CameraHwnd, SW_SHOW);
					break;
				}

				case 2:
				{
					ShowWindow(mli_CameraHwnd, SW_HIDE);
					SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
					mil_bCameraConnected = false;
					break;
				}

				case 3:
				{
					//Grab a Frame
					SendMessage(mli_CameraHwnd, WM_CAP_GRAB_FRAME, 0, 0);
					//Copy the frame we have just grabbed to the clipboard
					SendMessage(mli_CameraHwnd, WM_CAP_EDIT_COPY, 0, 0);
					//Copy the clipboard image data to a HBITMAP object called hbm
					HDC hdc = BeginPaint(mli_CameraHwnd, &ps);
					hdcMem = CreateCompatibleDC(hdc);

					if (hdcMem != NULL)
					{
						if (OpenClipboard(mli_CameraHwnd))
						{
							hbm = (HBITMAP)GetClipboardData(CF_BITMAP);
							SelectObject(hdcMem, hbm);
							GetClientRect(mli_CameraHwnd, &rc);
							CloseClipboard();
						}
					}

					//Save hbm to a .bmp file with date/time based name
					PBITMAPINFO pbi = CreateBitmapInfoStruct(hWnd, hbm);

					__time64_t long_time;
					struct tm newtime;
					wchar_t buffer[80];

					_time64(&long_time);
					_localtime64_s(&newtime, &long_time); // Convert to local time.
					int len = swprintf_s(buffer, 80, L"LI_%04d-%02d-%02d_%02d%02d.bmp", (newtime.tm_year + 1900), (newtime.tm_mon + 1), newtime.tm_mday, newtime.tm_hour, newtime.tm_min);

					if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Creating File: LI_%04d-%02d-%02d_%02d%02d.bmp\n", (newtime.tm_year + 1900), (newtime.tm_mon + 1), newtime.tm_mday, newtime.tm_hour, newtime.tm_min) > 0))
						OutputDebugString(mil_DbgMesg);

					// if(CreateBMPFile(hWnd, buffer, pbi, hbm, hdcMem))
					CreateBMPFile(hWnd, buffer, pbi, hbm, hdcMem);
					// loadImage("Sample1.bmp", &WorkimageDC, &image1Bmp);
					// loadImage("Sample1.bmp", &hdc, &image0Bmp);
					mil_bDrawImage = LoadBitmapFromBMPFile(L"Sample1.bmp", &image0Bmp, &hPalette);

					if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Result from LoadBitmapFromBMPFile: %s\n", (mil_bDrawImage ? L"Success":L"Failure")) > 0))
						OutputDebugString(mil_DbgMesg);

					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);

					// UpdateWindow(mli_CameraHwnd);
					// SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
					// SendMessage(mli_CameraHwnd, WM_PAINT, 0, 0);
					// ProcessImage(buffer);
					// drawImage(hdc, &WorkimageDC);
					/*
					SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
					SendMessage(mli_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
					SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
					SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0); */
					break;
				} // End case 3:
			} // End switch (wmId)
		} // End case WM_COMMAND:
        break;
    case WM_PAINT:
        {
			BITMAP bm;
            PAINTSTRUCT ps;

			HBITMAP       hOldBitmap;
			HPALETTE      hOldPalette;

            HDC hdc = BeginPaint(hWnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);
            // TODO: Add any drawing code that uses hdc here...
			if (mil_bDrawImage)
			{
				HDC hdcCam = BeginPaint(mli_CameraHwnd, &ps);
				HDC hdcMemCam = CreateCompatibleDC(hdcCam);

				GetObject(image0Bmp, sizeof(BITMAP), &bm);
				hOldBitmap = (HBITMAP)SelectObject(hdcMemCam, image0Bmp);
				hOldPalette = SelectPalette(hdcCam, hPalette, FALSE);
				RealizePalette(hdcCam);

				BitBlt(hdcCam, 0, 0, bm.bmWidth, bm.bmHeight, hdcMemCam, 0, 0, SRCCOPY);
				SelectObject(hdcMemCam, hOldBitmap);
				// DeleteObject(hBitmap);
				SelectPalette(hdcCam, hOldPalette, FALSE);
				// DeleteObject(hPalette);
				DeleteDC(hdcMemCam);
				EndPaint(mli_CameraHwnd, &ps);
			}
			// drawImage(hdc, &WorkimageDC);                  // draw our image to our screen DC
			DeleteDC(hdcMem);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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

bool CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{

	HANDLE hf;                  // file handle
	BITMAPFILEHEADER hdr;       // bitmap file-header
	PBITMAPINFOHEADER pbih;     // bitmap info-header
	LPBYTE lpBits;              // memory pointer
	DWORD dwTotal;              // total count of bytes
	DWORD cb;                   // incremental count of bytes
	BYTE *hp;                   // byte pointer
	DWORD dwTmp;
	bool bErrorOccured = false;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits)
	{
		MessageBox(hwnd, L"GlobalAlloc", L"Error", MB_OK);
		bErrorOccured = true;
	}

	// Retrieve the color table (RGBQUAD array) and the bits
	// (array of palette indices) from the DIB.
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
	{
		MessageBox(hwnd, L"GetDIBits", L"Error", MB_OK);
		bErrorOccured = true;
	}

	// Create the .BMP file.
	hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	if (hf == INVALID_HANDLE_VALUE)
	{
		MessageBox(hwnd, L"CreateFile", L"Error", MB_OK);
		bErrorOccured = true;
	}

	hdr.bfType = 0x4d42;  // File type designator "BM" 0x42 = "B" 0x4d = "M"
						  // Compute the size of the entire file.
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	// Compute the offset to the array of color indices.
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file.
	if (!WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER), (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteFileHeader", L"Error", MB_OK);
		bErrorOccured = true;
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteInfoHeader", L"Error", MB_OK);
		bErrorOccured = true;
	}

	// Copy the array of color indices into the .BMP file.
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;

	if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteFile", L"Error", MB_OK);
		bErrorOccured = true;
	}

	// Close the .BMP file.
	if (!CloseHandle(hf))
	{
		MessageBox(hwnd, L"CloseHandle", L"Error", MB_OK);
		bErrorOccured = true;
	}

	// Free memory.
	GlobalFree((HGLOBAL)lpBits);
	return bErrorOccured;
}

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD cClrBits;

	// Retrieve the bitmap color format, width, and height.
	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
	{
		MessageBox(hwnd, L"GetObject", L"Error", MB_OK);
	}

	// Convert the color format to a count of bits.
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. (This structure
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD
	// data structures.)
	if (cClrBits != 24)
	{
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * ((int)(1 << cClrBits)));
	}
	else // There is no RGBQUAD array for the 24-bit-per-pixel format.
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

	// Initialize the fields in the BITMAPINFO structure.
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;

	if (cClrBits < 24)
	{
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);
	}

	// If the bitmap is not compressed, set the BI_RGB flag.
	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color
	// indices and store the result in biSizeImage.
	// For Windows NT, the width must be DWORD aligned unless
	// the bitmap is RLE compressed. This example shows this.
	// For Windows 95/98/Me, the width must be WORD aligned unless the
	// bitmap is RLE compressed.
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;

	// Set biClrImportant to 0, indicating that all of the
	// device colors are important.
	pbmi->bmiHeader.biClrImportant = 0;

	return pbmi; //return BITMAPINFO
}


HRESULT EnumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
	// Create the System Device Enumerator.
	ICreateDevEnum *pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;
}

void Get_DeviceInfo()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		IEnumMoniker *pEnum;

		hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
		if (SUCCEEDED(hr))
		{
			// DisplayDeviceInformation(pEnum);
			pEnum->Release();
		}
		hr = EnumerateDevices(CLSID_AudioInputDeviceCategory, &pEnum);
		if (SUCCEEDED(hr))
		{
			// DisplayDeviceInformation(pEnum);
			pEnum->Release();
		}
		CoUninitialize();
	}
}

// **************************************************************************************

static void setcolor(unsigned int color)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, color | FOREGROUND_INTENSITY);
}

void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}



/*
* Do something with the filter. In this sample we just test the pan/tilt properties.
*/
void process_filter(IBaseFilter *pBaseFilter, int iIndex)
{
	CamCaps(pBaseFilter, iIndex);
}

HRESULT CamCaps(IBaseFilter *pBaseFilter, int iIndex)
{
	HRESULT hr = 0;
	vector <IPin*> pins;
	IEnumPins *EnumPins;
	IEnumMediaTypes *emt = NULL;
	AM_MEDIA_TYPE *pmt;
	vector<SIZE> modes;

	pBaseFilter->EnumPins(&EnumPins);
	pins.clear();
	for (;;)
	{
		IPin *pin;
		hr = EnumPins->Next(1, &pin, NULL);
		if (hr != S_OK) { break; }
		pins.push_back(pin);
		pin->Release();
	}
	EnumPins->Release();

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Number of Device pins: %zd\n", pins.size()) > 0))
		OutputDebugString(mil_DbgMesg);

	PIN_INFO pInfo;
	for (int i = 0; i<pins.size(); i++)
	{
		pins[i]->QueryPinInfo(&pInfo);
		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Pin name: %s \n", pInfo.achName) > 0))
			OutputDebugString(mil_DbgMesg);

		pins[i]->EnumMediaTypes(&emt);
		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Avialable resolutions for: %ls \n", mfg_CameraNames[iIndex]) >= 1))
			OutputDebugString(mil_DbgMesg);

		for (;;)
		{
			hr = emt->Next(1, &pmt, NULL);
			if (hr != S_OK) { break; }

			if ((pmt->formattype == FORMAT_VideoInfo) && (pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) && (pmt->pbFormat != NULL))
			{
				VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pmt->pbFormat;
				SIZE s;
				// Get frame size
				s.cy = pVIH->bmiHeader.biHeight;
				s.cx = pVIH->bmiHeader.biWidth;
				unsigned int bitrate = pVIH->dwBitRate;
				modes.push_back(s);
				// Bits per pixel
				unsigned int bitcount = pVIH->bmiHeader.biBitCount;
				REFERENCE_TIME t = pVIH->AvgTimePerFrame; // blocks (100ns) per frame
				int FPS = (int)(floor(10000000.0 / static_cast<double>(t)));
				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Size: x=%d\ty=%d\tFPS: %d\t bitrate: %ld\tbit/pixel:%ld\n", s.cx, s.cy, FPS, bitrate, bitcount) >= 1))
					OutputDebugString(mil_DbgMesg);
			}
			_FreeMediaType(*pmt);
		}
		modes.clear();
		emt->Release();
	}
	pins.clear();
	return S_OK;
}// End CamCaps()

///////////////////////////////
///////////////////////////////
// Function to load the image into our DC so we can draw it to the screen
void loadImage(const char* pathname, HDC *imageDC, HBITMAP *imageBmp)
{
	*imageDC = CreateCompatibleDC(NULL);     // create an offscreen DC

	*imageBmp = (HBITMAP)LoadImageA(         // load the bitmap from a file
		NULL,                           // not loading from a module, so this is NULL
		pathname,                       // the path we're loading from
		IMAGE_BITMAP,                   // we are loading a bitmap
		0, 0,                            // don't need to specify width/height
		LR_DEFAULTSIZE | LR_LOADFROMFILE// use the default bitmap size (whatever the file is), and load it from a file
	);

	imageBmpOld = (HBITMAP)SelectObject(*imageDC, *imageBmp);  // put the loaded image into our DC
}


///////////////////////////////
// Function to clean up
void cleanUpImage(HDC *imageDC, HBITMAP *imageBmp)
{
	SelectObject(*imageDC, imageBmpOld);      // put the old bmp back in our DC
	DeleteObject(*imageBmp);                 // delete the bmp we loaded
	DeleteDC(*imageDC);                      // delete the DC we created
}

///////////////////////////////
///////////////////////////////
// The function to draw our image to the display (the given DC is the screen DC)
void drawImage(HDC screen, HDC *imageDC)
{
	BitBlt(
		screen,         // tell it we want to draw to the screen
		0, 0,            // as position 0,0 (upper-left corner)
		CANERA_WIDTH,   // width of the rect to draw
		CANERA_HEIGHT,   // height of the rect
		*imageDC,        // the DC to get the rect from (our image DC)
		0, 0,            // take it from position 0,0 in the image DC
		SRCCOPY         // tell it to do a pixel-by-pixel copy
	);
}

/*
* Enumerate all video devices
*
* See also:
*
* Using the System Device Enumerator:
*     http://msdn2.microsoft.com/en-us/library/ms787871.aspx
*/
int enum_devices()
{
	HRESULT hr;
	int NumCamerasFound = 0;
	

	OutputDebugString(L"Enumeraring videoinput devices ...\n");

	CoInitialize(NULL);
	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		OutputDebugString(L"Error. Can't create enumerator.\n");
		return hr;
	}

	// Obtain a class enumerator for the video input device category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK)
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;

		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			NumCamerasFound++; // We found a camera
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// Retrieve the device's friendly name
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0); // Read the device's name
				if (SUCCEEDED(hr))
				{
					if (NumCamerasFound < MAX_CAMERAS)
					{
						// Record the name, indexed by order found
						mfg_CameraNames[NumCamerasFound - 1] = new wchar_t[MAX_LOADSTRING];
						wcsncpy_s(mfg_CameraNames[NumCamerasFound - 1], MAX_LOADSTRING, varName.bstrVal, _TRUNCATE);
						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Found Camera: %ls\n", mfg_CameraNames[NumCamerasFound - 1]) >= 1))
							OutputDebugString(mil_DbgMesg);
					}
				}
				VariantClear(&varName);
				// To create an instance of the filter, do the following:
				IBaseFilter *pFilter;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
				process_filter(pFilter, (NumCamerasFound - 1));
				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
	OleUninitialize();
	return NumCamerasFound;
}

BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP *phBitmap, HPALETTE *phPalette)
{

	BITMAP  bm;

	*phBitmap = NULL;
	*phPalette = NULL;

	// Use LoadImage() to get the image loaded into a DIBSection
	*phBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0,LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	if (*phBitmap == NULL)
		return FALSE;

	// Get the color depth of the DIBSection
	GetObject(*phBitmap, sizeof(BITMAP), &bm);
	// If the DIBSection is 256 color or less, it has a color table
	if ((bm.bmBitsPixel * bm.bmPlanes) <= 8)
	{
		HDC           hMemDC;
		HBITMAP       hOldBitmap;
		RGBQUAD       rgb[256];
		LPLOGPALETTE  pLogPal;
		WORD          i;

		// Create a memory DC and select the DIBSection into it
		hMemDC = CreateCompatibleDC(NULL);
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, *phBitmap);
		// Get the DIBSection's color table
		GetDIBColorTable(hMemDC, 0, 256, rgb);
		// Create a palette from the color tabl
		pLogPal = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + (256 * sizeof(PALETTEENTRY)));
		pLogPal->palVersion = 0x300;
		pLogPal->palNumEntries = 256;
		for (i = 0; i<256; i++)
		{
			pLogPal->palPalEntry[i].peRed = rgb[i].rgbRed;
			pLogPal->palPalEntry[i].peGreen = rgb[i].rgbGreen;
			pLogPal->palPalEntry[i].peBlue = rgb[i].rgbBlue;
			pLogPal->palPalEntry[i].peFlags = 0;
		}
		*phPalette = CreatePalette(pLogPal);
		// Clean up
		free(pLogPal);
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
	}
	else   // It has no color table, so use a halftone palette
	{
		HDC    hRefDC;

		hRefDC = GetDC(NULL);
		*phPalette = CreateHalftonePalette(hRefDC);
		ReleaseDC(NULL, hRefDC);
	}
	return TRUE;

}
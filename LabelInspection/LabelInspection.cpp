/* *************************************************************************************************
* FrameGrab  - Created: 08/11/2016  8/11/2016 3:25:59 PM
* Creator Robert K Young - rkyoung@sonic.net
* ChangeLog:
* 0.0.1 - 8/11/2016 3:32:33 PM - Initial Version
* 0.0.2 - 8/12/2016 5:22:19 PM - Filled in skeleton, corrected typos in code copied, and made added
*		  code Unicode  complaint.
* 0.0.3 - 8/14/2016 2:34:38 PM - Failed attempt to add functionality
* 0.0.4 - 8/15/2016 4:22:15 PM - Added capture file name based on date/time
* 0.1.0 - 8/16/2016 3:38:58 PM - First beta version with basic functionality
* 0.2.0 - 8/23/2016 1:36:01 PM - Incorporated OpenCV
//**************************************************************************************************/



// --------------------------------------------------------------------
// LabelInspection.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "LabelInspection.h"
#include <Strsafe.h> 
#include <stdio.h>
#include <windows.h>  
#include <vfw.h>  
#include <time.h>
#include <wchar.h>
#include <cwchar>
#include <dshow.h>
#include <vector>

// OpenCV
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\imgcodecs\imgcodecs.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <iostream>

#define CANERA_WIDTH 1288 
#define CANERA_HEIGHT 964 

using namespace cv;
using namespace std;

#define BLUE    0x0001
#define GREEN   0x0002
#define RED     0x0004
#define GRAY    0x0007	

//Remember to Link to vfw32 Library, gdi32 Library  

#define BLUE    0x0001
#define GREEN   0x0002
#define RED     0x0004
#define GRAY    0x0007	

// Contributing Source Used: http://www.dreamincode.net/forums/topic/193519-win32-webcam-program/
//

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
HWND hWindow;
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
LPCTSTR szAppName = L"FrameGrab";
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
void Get_DeviceInfo();
int enum_devices();
void process_filter(IBaseFilter *pBaseFilter);
HRESULT CamCaps(IBaseFilter *pBaseFilter);
void _FreeMediaType(AM_MEDIA_TYPE& mt);
static void setcolor(unsigned int color);

HWND camhwnd;
bool bCameraConnected = false;
HDC hdc;
HDC hdcMem;
PAINTSTRUCT ps;
HBITMAP hbm;
RECT rc;



#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR    lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LABELINSPECTION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    // HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABELINSPECTION));

	int result;
	// Get_DeviceInfo();
	// result = enum_devices();

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
ATOM MyRegisterClass(HINSTANCE hInstance)
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

	/***********************************************************************

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = szAppName;
	RegisterClass(&wc);

	**********************************************************************/

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
   int WindowX_Dim = (GetSystemMetrics(SM_CXSCREEN) - (GetSystemMetrics(SM_CXSCREEN) >> 4));
   int WindowY_Dim = (GetSystemMetrics(SM_CYSCREEN) - (GetSystemMetrics(SM_CYSCREEN) >> 4));

   // hWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, hInstance, nullptr);
   hWindow = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, WindowX_Dim, WindowY_Dim, nullptr, nullptr, hInstance, nullptr);

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
		SetBkMode(hdc, TRANSPARENT);
		return (LRESULT)CreateSolidBrush(0xFFFFFF);

	case WM_CREATE:
	{
		hButtStartCam = CreateWindowEx(0, L"BUTTON", L"Start Camera", WS_CHILD | WS_VISIBLE, 0, 0, 300, 60, hWnd, (HMENU)1, hInstance, 0);
		hButtStopCam = CreateWindowEx(0, L"BUTTON", L"Stop Camera", WS_CHILD | WS_VISIBLE, 0, 75, 300, 60, hWnd, (HMENU)2, hInstance, 0);
		hButtGrabFrame = CreateWindowEx(0, L"BUTTON", L"Snaphot", WS_CHILD | WS_VISIBLE, 0, 150, 300, 60, hWnd, (HMENU)3, hInstance, 0);
		
		// CANERA_WIDTH 1288 
		// CANERA_HEIGHT 964 


		// Get the desktop dims and take a little off all four sides
		int camWindowX_Dim = (CANERA_WIDTH - (CANERA_WIDTH >> 4));
		int camWindowY_Dim = (GetSystemMetrics(SM_CYSCREEN) - (GetSystemMetrics(SM_CYSCREEN) >> 4));

		// camhwnd = capCreateCaptureWindow(L"camera window", WS_CHILD, 400, 25, 640, 480, hWnd, 0);
		camhwnd = capCreateCaptureWindow(L"camera window", WS_CHILD, 301, 25, CANERA_WIDTH, CANERA_HEIGHT, hWnd, 0);
		// bCameraConnected = false;
		// SendMessage(camhwnd, WM_CAP_DLG_VIDEOSOURCE, 0, 0);
		// if (!bCameraConnected)
			bCameraConnected = SendMessage(camhwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
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
				// if (!bCameraConnected)
					bCameraConnected = SendMessage(camhwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
				SendMessage(camhwnd, WM_CAP_SET_SCALE, true, 0);
				SendMessage(camhwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
				SendMessage(camhwnd, WM_CAP_SET_PREVIEW, true, 0);
				ShowWindow(camhwnd, SW_SHOW);
				break;
			}

			case 2:
			{
				ShowWindow(camhwnd, SW_HIDE);
				SendMessage(camhwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
				bCameraConnected = false;
				break;
			}

			case 3:
			{
				//Grab a Frame
				SendMessage(camhwnd, WM_CAP_GRAB_FRAME, 0, 0);
				//Copy the frame we have just grabbed to the clipboard
				SendMessage(camhwnd, WM_CAP_EDIT_COPY, 0, 0);
				//Copy the clipboard image data to a HBITMAP object called hbm
				hdc = BeginPaint(camhwnd, &ps);
				hdcMem = CreateCompatibleDC(hdc);

				if (hdcMem != NULL)
				{
					if (OpenClipboard(camhwnd))
					{
						hbm = (HBITMAP)GetClipboardData(CF_BITMAP);
						SelectObject(hdcMem, hbm);
						GetClientRect(camhwnd, &rc);
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
				int len = swprintf_s(buffer, 80, L"FG_%04d-%02d-%02d_%02d%02d.bmp", (newtime.tm_year + 1900), (newtime.tm_mon + 1), newtime.tm_mday, newtime.tm_hour, newtime.tm_min);

				CreateBMPFile(hWnd, buffer, pbi, hbm, hdcMem);
				SendMessage(camhwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
				SendMessage(camhwnd, WM_CAP_SET_SCALE, true, 0);
				SendMessage(camhwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
				SendMessage(camhwnd, WM_CAP_SET_PREVIEW, true, 0);
				break;
			}
			

			// *****************************************************


            // default:
            //    return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
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

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{

	HANDLE hf;                  // file handle
	BITMAPFILEHEADER hdr;       // bitmap file-header
	PBITMAPINFOHEADER pbih;     // bitmap info-header
	LPBYTE lpBits;              // memory pointer
	DWORD dwTotal;              // total count of bytes
	DWORD cb;                   // incremental count of bytes
	BYTE *hp;                   // byte pointer
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits)
	{
		MessageBox(hwnd, L"GlobalAlloc", L"Error", MB_OK);
	}

	// Retrieve the color table (RGBQUAD array) and the bits
	// (array of palette indices) from the DIB.
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
	{
		MessageBox(hwnd, L"GetDIBits", L"Error", MB_OK);
	}

	// Create the .BMP file.
	hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	if (hf == INVALID_HANDLE_VALUE)
	{
		MessageBox(hwnd, L"CreateFile", L"Error", MB_OK);
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
	}

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
	if (!WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD), (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteInfoHeader", L"Error", MB_OK);
	}

	// Copy the array of color indices into the .BMP file.
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;

	if (!WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL))
	{
		MessageBox(hwnd, L"WriteFile", L"Error", MB_OK);
	}

	// Close the .BMP file.
	if (!CloseHandle(hf))
	{
		MessageBox(hwnd, L"CloseHandle", L"Error", MB_OK);
	}

	// Free memory.
	GlobalFree((HGLOBAL)lpBits);
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
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << cClrBits));
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
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

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


HRESULT CamCaps(IBaseFilter *pBaseFilter)
{
	HRESULT hr = 0;
	vector <IPin*> pins;
	IEnumPins *EnumPins;
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

	printf("Device pins number: %zd\n", pins.size());

	PIN_INFO pInfo;
	for (int i = 0; i<pins.size(); i++)
	{
		pins[i]->QueryPinInfo(&pInfo);

		setcolor(RED);

		if (pInfo.dir == 0)
		{
			wprintf(L"Pin name: %s \n", pInfo.achName);
		}

		if (pInfo.dir == 1)
		{
			wprintf(L"Pin name: %s \n", pInfo.achName);
		}

		IEnumMediaTypes *emt = NULL;
		pins[i]->EnumMediaTypes(&emt);

		AM_MEDIA_TYPE *pmt;

		vector<SIZE> modes;
		setcolor(GRAY);
		wprintf(L"Avialable resolutions: %s \n", pInfo.achName);
		for (;;)
		{
			hr = emt->Next(1, &pmt, NULL);
			if (hr != S_OK) { break; }

			if ((pmt->formattype == FORMAT_VideoInfo) &&
				//(pmt->subtype == MEDIASUBTYPE_RGB24) &&
				(pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
				(pmt->pbFormat != NULL))
			{
				VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pmt->pbFormat;
				SIZE s;
				// Get frame size
				s.cy = pVIH->bmiHeader.biHeight;
				s.cx = pVIH->bmiHeader.biWidth;
				// 
				unsigned int bitrate = pVIH->dwBitRate;
				modes.push_back(s);
				// Bits per pixel
				unsigned int bitcount = pVIH->bmiHeader.biBitCount;
				REFERENCE_TIME t = pVIH->AvgTimePerFrame; // blocks (100ns) per frame
				int FPS = floor(10000000.0 / static_cast<double>(t));
				printf("Size: x=%d\ty=%d\tFPS: %d\t bitrate: %ld\tbit/pixel:%ld\n", s.cx, s.cy, FPS, bitrate, bitcount);
			}
			_FreeMediaType(*pmt);
		}
		//----------------------------------------------------
		// 
		// 
		// 
		//----------------------------------------------------
		modes.clear();
		emt->Release();
	}

	pins.clear();

	return S_OK;
}

/*
* Do something with the filter. In this sample we just test the pan/tilt properties.
*/
void process_filter(IBaseFilter *pBaseFilter)
{
	CamCaps(pBaseFilter);
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
	setcolor(GRAY);
	printf("Enumeraring videoinput devices ...\n");

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		fprintf(stderr, "Error. Can't create enumerator.\n");
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
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					setcolor(GREEN);
					wprintf(L"------------------> %s <------------------\n", varName.bstrVal);
				}
				VariantClear(&varName);

				// To create an instance of the filter, do the following:
				IBaseFilter *pFilter;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);

				process_filter(pFilter);

				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();

	return 0;
}

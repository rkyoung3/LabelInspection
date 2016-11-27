/* *************************************************************************************************
* LabelInspection  - Created: 08/23/2016  3:25:59 PM
* Creator Robert K Young - rkyoung@sonic.net
* ChangeLog:
* 0.0.1 - 08/23/2016 03:32:33 PM - Initial Version
* 0.0.2 - 08/24/2016 04:22:15 PM - Added capture file name based on date/time
* 0.1.0 - 08/15/2016 03:38:58 PM - First beta version with basic functionality
* 0.2.0 - 08/25/2016 01:36:01 PM - Incorporated OpenCV
* 0.3.0 - 09/01/2016 05:21:43 PM - Began implementing MediaFoundation calls to control camera.
* 0.4.0 - 09/13/2016 09:11:15 AM - Implementation of MediaFoundation failed, moving on to processing image
* 0.5.0 - 10/15/2016 11:06:10 AM - Successfully locate all ref marks on simulated data
* 0.6.0 - 11/04/2016 12:51:16 AM - Successfully create Golden Image from all ref marks found
* 0.7.0 - 11/24/2016 11:18:32 AM - Final Registration Mark version (Changing to border detection).
* 0.8.0 - 11/27/2016 11:06:18 AM - Begin implementing BorderDetection
//************************************************************************************************* 
*/

#include "stdafx.h"

// #define POINT_GREY_CAMERA 1

#define CANERA_WIDTH 1288 
#define CANERA_HEIGHT 964 


using namespace cv;
using namespace std;

namespace std { 
#include <cstdlib> 
};


// Contributing Source Used: http://www.dreamincode.net/forums/topic/193519-win32-webcam-program/

HWND mil_hWindow;
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
bool CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);
LPCTSTR szAppName = L"FrameGrab";
void drawImage(HDC screen, HDC *imageDC);
void Get_DeviceInfo();
int enum_devices();
void process_filter(IBaseFilter *pBaseFilter, int iIndex);
HRESULT CamCaps(IBaseFilter *pBaseFilter, int iIndex);
void _FreeMediaType(AM_MEDIA_TYPE& mt);
static void setcolor(unsigned int color);
void loadImage(const char* pathname, HDC *imageDC, HBITMAP *imageBmp);
void cleanUpImage(HDC *imageDC, HBITMAP *imageBmp);
BOOL LoadBitmapFromBMPFile(LPTSTR szFileName, HBITMAP *phBitmap, HPALETTE *phPalette);
DWORD WINAPI SetupThreadProc(_In_ LPVOID lpParameter);
BOOL bPointIsInRect(POINT Candidate, RECT Area);
BOOL GetClientScreenCoordinates(HWND hWindowParent, HWND hWindowChild, RECT *pRect);
void Screen2Client(POINT *MousePosition);
UINT LocateRefMarks(RECT *CaptureRect, HDC RMarkHandel);
BOOL isMatchToRefColumn(RGBTRIPLE *RefValues, RGBTRIPLE *CandidatePixel, UINT Increment, UINT NumberOfValues, BYTE rgbTolerance);
BOOL isMatchToRefRow(RGBTRIPLE *RefValues, RGBTRIPLE *CandidatePixel, UINT NumberOfValues, BYTE rgbTolerance);
BOOL rectIsDuplicate(RECT *RefMarkRects, RECT Candidate, UINT MarksFound);
bool CreateReferenceImage(RGBTRIPLE *FullImage, UINT FullZoneWidth, UINT FullZoneHeight);

HWND mil_hButtonChangeCam;
HWND mil_hButtonRefImage;
HWND mli_CameraHwnd;
int mil_CameraCount;
int mli_WindowX_Dim;
int mli_WindowY_Dim;
int mli_CamWindowX_Dim;
int mli_CamWindowY_Dim;
float mil_Dim_Multiplier;
float mil_Dim_Conversion;
bool mil_bCameraConnected = false;
PBITMAPINFO mil_pImageBitMapInfo;
LPBYTE mil_pImageBits; 
RGBTRIPLE *mil_RefImage;
wchar_t* mil_CameraNames[MAX_CAMERAS];
RECT mil_RectsDefined[MAX_RECTANGLES];
RECT *mil_RefMarkRects = NULL;
RECT *mil_ZoneRects = NULL;
UINT mil_ToDraw = 0;
ProcessImage *mil_ProcessImage;
// The Inspection area is defined as the opposite 
// corner from the down point that defined the Rectangle
// of the RefMark.
UINT mil_RectsToDraw = 0;
bool bBeginDefiningRefMark = false;
bool bDefiningRefMark = false;
bool bRefMarkSearched = false;
bool bDrawFoundRefMark = false;
bool mil_bRefMarksDrawn = false;
bool mil_bDefiningInspectionZone = false;
bool bDefiningInspectionZone = false;
POINT mil_DownPointRefMark;
POINT mil_UpPointRefMark;
POINT mil_InspectionArea;
wchar_t mil_DbgMesg[MAX_PATH];
HWND hUserMsg;
HDC mil_hdcMem;
RECT mil_MainClientRect;
RECT mil_CamClientRect;
HDC         WorkimageDC;        // the DC to hold our image
HBITMAP     hbmp_image0Bmp;       // the bitmap which contains the image we are working on
HPALETTE      hPalette;
HBITMAP     imageBmpOld;    // the DC's old bitmap (for cleanup)
HBITMAP     OriginalimageBmp;
RGBTRIPLE *mil_SubImage;

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
	mil_CameraCount = enum_devices();

	if(mil_CameraCount == 0)
		return FALSE;

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
		if (!IsDialogMessage(mil_hWindow, &msg))
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
	wcex.hCursor		= NULL; // LoadCursor(nullptr, IDC_ARROW);
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
   mli_WindowX_Dim = GetSystemMetrics(SM_CXSCREEN); // -(GetSystemMetrics(SM_CXSCREEN) >> 4));
   mli_WindowY_Dim = GetSystemMetrics(SM_CYSCREEN); // -(GetSystemMetrics(SM_CYSCREEN) >> 4));

   DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

   if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Creating MainWindow: %04d x %04d Style: 0x%08X\n", mli_WindowX_Dim, mli_WindowY_Dim, dwStyle) > 0))
	   OutputDebugString(mil_DbgMesg);

   mil_hWindow = CreateWindowW(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, mli_WindowX_Dim, mli_WindowY_Dim, nullptr, nullptr, hInstance, nullptr);

   if (!mil_hWindow)
   {
      return FALSE;
   }

   ShowWindow(mil_hWindow, nCmdShow);
   UpdateWindow(mil_hWindow);

   return TRUE;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWindow, UINT WndMessage, WPARAM wParam, LPARAM lParam)
{

	HINSTANCE hInstance = GetModuleHandle(NULL);

	switch (WndMessage)
	{
	case WM_CTLCOLORSTATIC:
		SetBkMode(mil_hdcMem, TRANSPARENT);
		return (LRESULT)CreateSolidBrush(0xFFFFFF);

	case WM_CREATE:
	{

		// If we have more than one camera
		// add a change camera button
		if (mil_CameraCount > 1)
		{
			mil_hButtonChangeCam = CreateWindowEx(0, L"BUTTON", L"Change Camera", WS_CHILD | WS_VISIBLE, 0, 0, WIDTH_CAMERA_EXCLUDE, 60, hWindow, (HMENU) MENU_BUTTON_CHANGECAM, hInstance, 0);
			mil_hButtonRefImage = CreateWindowEx(0, L"BUTTON", L"Create Ref Image", WS_CHILD | WS_VISIBLE, 0, 75, WIDTH_CAMERA_EXCLUDE, 60, hWindow, (HMENU) MENU_BUTTON_REF_IMAGE, hInstance, 0);
			hUserMsg = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Status Messages", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_BORDER, 0, 150, WIDTH_CAMERA_EXCLUDE, 200, hWindow, (HMENU)MENU_STAT_WINDOW, hInstance, NULL);
		}else{
			mil_hButtonRefImage = CreateWindowEx(0, L"BUTTON", L"Create Ref Image", WS_CHILD | WS_VISIBLE, 0, 0, WIDTH_CAMERA_EXCLUDE, 60, hWindow, (HMENU)MENU_BUTTON_REF_IMAGE, hInstance, 0);
			hUserMsg = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Status Messages", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_BORDER, 0, 150, WIDTH_CAMERA_EXCLUDE, 200, hWindow, (HMENU)MENU_STAT_WINDOW, hInstance, NULL);
		}


		HGDIOBJ hfDefault = GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(hUserMsg, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
		SendMessage(hUserMsg, WM_SETTEXT, NULL, (LPARAM)L"Status Messages:");

		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER; // | WS_HSCROLL | WS_VSCROLL;

		// Calc how much we need to shrink the width to fit the screen
		float XDim_Multiplier = 1.0;
		if ( (mli_WindowX_Dim - WIDTH_CAMERA_EXCLUDE) < CANERA_WIDTH )
		{
			float FullWidth = (float) CANERA_WIDTH;
			float MaxWidth = (float) (mli_WindowX_Dim - WIDTH_CAMERA_EXCLUDE);
			XDim_Multiplier = (MaxWidth / FullWidth);
		}

		// Calc how much we need to shrink the height to fit the screen
		float YDim_Multiplier = 1.0;
		if ( (mli_WindowY_Dim - HEIGHT_CAMERA_EXCLUDE) < CANERA_HEIGHT )
		{
			float FullHeight = (float) CANERA_HEIGHT;
			float MaxHeight = (float) (mli_WindowY_Dim - HEIGHT_CAMERA_EXCLUDE);
			YDim_Multiplier = (MaxHeight / FullHeight);
		}

		// Choose the smallest of the two multipliers and use it on 
		// both dimensions so that we maintain a correct aspect ratio
		mil_Dim_Multiplier = min(XDim_Multiplier, YDim_Multiplier);

		mli_CamWindowX_Dim = (int)(((float)(CANERA_WIDTH)) * mil_Dim_Multiplier);
		mli_CamWindowY_Dim = (int)(((float)(CANERA_HEIGHT)) * mil_Dim_Multiplier);

		// Calc the exact inverse so that screen/image 
		// conversions are correct in both directions
		mil_Dim_Conversion = ((float) 1.0) + ( 1.0 - mil_Dim_Multiplier);

		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Window Translate - Mult: %1.6f  Cnvt: %1.6f\n", mil_Dim_Multiplier, mil_Dim_Conversion) > 0))
			OutputDebugString(mil_DbgMesg);

		mli_CameraHwnd = capCreateCaptureWindow(L"camera window", dwStyle, 301, 0, mli_CamWindowX_Dim, mli_CamWindowY_Dim, hWindow, 0);

		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Creating Camera Window: %04d x %04d\n", mli_CamWindowX_Dim, mli_CamWindowY_Dim) > 0))
			OutputDebugString(mil_DbgMesg);

		// Put the hardware into a known state
		SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
		// In multiple camera environments: Windows insists 
		// on handeling camera selection it will use whatever 
		// camera the user selects as camera 0
		SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
		SendMessage(mli_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
		SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
		SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
		ShowWindow(hWindow, SW_SHOW);
		break;
	} // End case WM_CREATE:

	case WM_LBUTTONDOWN:
	{
		POINT MousePosition;
		MousePosition.x = GET_X_LPARAM(lParam);
		MousePosition.y = GET_Y_LPARAM(lParam);

		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Reported Mouse Button Down Position - X: %04d Y: %04d\n", MousePosition.x, MousePosition.y) > 0))
			OutputDebugString(mil_DbgMesg);

		if (bPointIsInRect(MousePosition, mil_CamClientRect))
		{
			// ScreenToClient(mli_CameraHwnd, &MousePosition);
			Screen2Client(&MousePosition);

			if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"After Left Button Down Position - X: %04d Y: %04d\n", MousePosition.x, MousePosition.y) > 0))
				OutputDebugString(mil_DbgMesg);

			if (mil_bDefiningInspectionZone)
			{
				mil_bDefiningInspectionZone = false;
				bBeginDefiningRefMark = false;
				bDefiningRefMark = true;
				mil_InspectionArea = MousePosition;
				SetRect(&mil_RectsDefined[ZONE_RECTANGLE], mil_DownPointRefMark.x, mil_DownPointRefMark.y, mil_InspectionArea.x, mil_InspectionArea.y);
				ReleaseCapture();
				mil_RectsToDraw = 2;
				bDrawFoundRefMark = true;
				RedrawWindow(hWindow, NULL, NULL, RDW_INVALIDATE);
			}

			if (bBeginDefiningRefMark)  
			{
				bBeginDefiningRefMark = false;
				bDefiningRefMark = true;
				mil_DownPointRefMark = MousePosition;

				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Left Button Down Position - X: %04d Y: %04d\n", mil_DownPointRefMark.x, mil_DownPointRefMark.y) > 0))
					OutputDebugString(mil_DbgMesg);

				HCURSOR hCursor = LoadCursor(NULL, IDC_CROSS);
				SetCursor(hCursor);
			} // End if (bBeginDefiningRefMark)

		}else{ // Button down outside camera window cancel any definitions in progress

			if (bBeginDefiningRefMark || bDefiningRefMark || mil_bDefiningInspectionZone)
			{
				bDefiningRefMark = FALSE;
				Button_Enable(mil_hButtonRefImage, TRUE);
				ReleaseCapture();
				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Left Button Down Outside Camera Window - X: %04d Y: %04d\n", MousePosition.x, MousePosition.y) > 0))
					OutputDebugString(mil_DbgMesg);
				if (mil_hdcMem != NULL)
					DeleteDC(mil_hdcMem);
				if (hbmp_image0Bmp != NULL)
					DeleteObject(hbmp_image0Bmp);
				mil_hdcMem = NULL;
				hbmp_image0Bmp = NULL;
				// Put the hardware into a known state
				SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);

				SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
				SendMessage(mli_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
				SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
				SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
			}
		}
		break;
	}

	case WM_SIZE:
	{
		return 0;
	}
	


	case WM_MOUSEMOVE:
	{
		POINT MousePosition;
		MousePosition.x = GET_X_LPARAM(lParam);
		MousePosition.y = GET_Y_LPARAM(lParam);

		if (bPointIsInRect(MousePosition, mil_CamClientRect))
		{
			// ScreenToClient(mli_CameraHwnd, &MousePosition);
			Screen2Client(&MousePosition);
			if (bDefiningRefMark)
			{
				mil_UpPointRefMark = MousePosition;
				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Ref Mark Mouse Move Position - X: %04d Y: %04d\n", MousePosition.x, MousePosition.y) > 0))
					OutputDebugString(mil_DbgMesg);
				SetRect(&mil_RectsDefined[REF_RECTANGLE], mil_DownPointRefMark.x, mil_DownPointRefMark.y, mil_UpPointRefMark.x, mil_UpPointRefMark.y);
				mil_RectsToDraw = 1;
				RedrawWindow(hWindow, NULL, NULL, RDW_INVALIDATE);
			}

			if (mil_bDefiningInspectionZone)
			{
				mil_InspectionArea = MousePosition;

				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Inspection Area - Mouse Move Position - X: %04d Y: %04d\n", MousePosition.x, MousePosition.y) > 0))
					OutputDebugString(mil_DbgMesg);
				SetRect(&mil_RectsDefined[ZONE_RECTANGLE], mil_DownPointRefMark.x, mil_DownPointRefMark.y, mil_InspectionArea.x, mil_InspectionArea.y);
				mil_RectsToDraw = 2;
				RedrawWindow(hWindow, NULL, NULL, RDW_INVALIDATE);
			}

		}

		break;
	}

	case WM_LBUTTONUP:
	{
		POINT MousePosition;
		MousePosition.x = GET_X_LPARAM(lParam);
		MousePosition.y = GET_Y_LPARAM(lParam);

			
		if (bPointIsInRect(MousePosition, mil_CamClientRect))
		{
			// ScreenToClient(mli_CameraHwnd, &MousePosition); 
			Screen2Client( &MousePosition);
			if (bDefiningRefMark)
			{
				bDefiningRefMark = FALSE;
				mil_bDefiningInspectionZone = true;
				mil_UpPointRefMark = MousePosition;

				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Ref Mark - Left Button Up Position - X: %04d Y: %04d\n", mil_UpPointRefMark.x, mil_UpPointRefMark.y) > 0))
					OutputDebugString(mil_DbgMesg);

				HCURSOR hCursor = LoadCursor(NULL, IDC_CROSS);
				SetCursor(hCursor);
				SendMessage(hUserMsg, WM_SETTEXT, NULL, (LPARAM)L"Click on opposite corner from RefMark to define the Inspection Area.");
				
				SetRect(&mil_RectsDefined[REF_RECTANGLE], mil_DownPointRefMark.x, mil_DownPointRefMark.y, mil_UpPointRefMark.x, mil_UpPointRefMark.y);
				
			} // End if (bDefiningRefMark)

		}else{ // Button up outside camera window cancel DefiningRefMark
			Button_Enable(mil_hButtonRefImage, TRUE);
			ReleaseCapture();
			if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Ref Mark - Left Button Up Outside Camera Window - X: %04d Y: %04d\n", MousePosition.x, MousePosition.y) > 0))
				OutputDebugString(mil_DbgMesg);
			if (mil_hdcMem != NULL)
				DeleteDC(mil_hdcMem);
			if (hbmp_image0Bmp != NULL)
				DeleteObject(hbmp_image0Bmp);
			mil_hdcMem = NULL;
			hbmp_image0Bmp = NULL;
			HCURSOR hCursor = LoadCursor(NULL, IDC_ARROW);
			SetCursor(hCursor);
			// Put the hardware into a known state
			SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
			SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
			SendMessage(mli_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
			SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
			SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
		}
		break;
	}  // End case WM_LBUTTONUP:

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{

			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWindow, About);
				break;

			case IDM_EXIT:
				DestroyWindow(hWindow);
				break;

			case MENU_BUTTON_CHANGECAM:
			{
				SendMessage(mli_CameraHwnd, WM_CAP_DLG_VIDEOSOURCE, 0, 0);
				// Put the hardware into a known state
				SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0);
				SendMessage(mli_CameraHwnd, WM_CAP_DRIVER_CONNECT, 0, 0);
				SendMessage(mli_CameraHwnd, WM_CAP_SET_SCALE, true, 0);
				SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEWRATE, 66, 0);
				SendMessage(mli_CameraHwnd, WM_CAP_SET_PREVIEW, true, 0);
				ShowWindow(mli_CameraHwnd, SW_SHOW);
				break;
			}

			case MENU_BUTTON_REF_IMAGE:
			{
				PAINTSTRUCT ps;
				//Grab a Frame
				SendMessage(mli_CameraHwnd, WM_CAP_GRAB_FRAME, 0, 0);
				//Copy the frame we have just grabbed to the clipboard
				SendMessage(mli_CameraHwnd, WM_CAP_EDIT_COPY, 0, 0);
				//Copy the clipboard image data to a HBITMAP object called hbm
				HDC hdc = BeginPaint(mli_CameraHwnd, &ps);
				if (hdc != NULL)
				{
					mil_hdcMem = CreateCompatibleDC(hdc);
					if (mil_hdcMem != NULL)
					{
	#ifdef POINT_GREY_CAMERA
						if (OpenClipboard(mli_CameraHwnd))
						{
							hbmp_image0Bmp = (HBITMAP)GetClipboardData(CF_BITMAP);
							SelectObject(mil_hdcMem, hbmp_image0Bmp);
							CloseClipboard();
						}else{
							DeleteDC(mil_hdcMem);
							mil_hdcMem = NULL;
							hbmp_image0Bmp = NULL;
						}
						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Result from OpenClipboard: %s\n", ((hbmp_image0Bmp != NULL) ? L"Success" : L"Failure")) > 0))
							OutputDebugString(mil_DbgMesg);
	#else
						if (LoadBitmapFromBMPFile(L"Sample4.bmp", &hbmp_image0Bmp, &hPalette))
						{
							SelectObject(mil_hdcMem, hbmp_image0Bmp);
						}else{
							DeleteDC(mil_hdcMem);
							mil_hdcMem = NULL;
							hbmp_image0Bmp = NULL;
						}

						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Result from LoadBitmapFromBMPFile: %s\n", ((hbmp_image0Bmp != NULL) ? L"Success" : L"Failure")) > 0))
							OutputDebugString(mil_DbgMesg);
	#endif
						if (hbmp_image0Bmp != NULL)
						{
							//Save hbm to a .bmp file with date/time based name
							mil_pImageBitMapInfo = CreateBitmapInfoStruct(mli_CameraHwnd, hbmp_image0Bmp);

							__time64_t long_time;
							struct tm newtime;
							wchar_t buffer[80];

							_time64(&long_time);
							_localtime64_s(&newtime, &long_time); // Convert to local time.
							// int len = swprintf_s(buffer, 80, L"LI_%04d-%02d-%02d_%02d%02d.bmp", (newtime.tm_year + 1900), (newtime.tm_mon + 1), newtime.tm_mday, newtime.tm_hour, newtime.tm_min);
							int len = swprintf_s(buffer, 80, L"Image.bmp");

							// if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Creating File: LI_%04d-%02d-%02d_%02d%02d.bmp\n", (newtime.tm_year + 1900), (newtime.tm_mon + 1), newtime.tm_mday, newtime.tm_hour, newtime.tm_min) > 0))
							if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Creating File: %s\n", buffer) > 0))
								OutputDebugString(mil_DbgMesg);

							// if(CreateBMPFile(hWnd, buffer, pbi, hbm, hdcMem))
							CreateBMPFile(mli_CameraHwnd, buffer, mil_pImageBitMapInfo, hbmp_image0Bmp, mil_hdcMem);

							Button_Enable(mil_hButtonRefImage, FALSE);
							if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Disabled Create Ref Image Button\n") >= 1))
								OutputDebugString(mil_DbgMesg);

							GetClientScreenCoordinates(mil_hWindow, mli_CameraHwnd, &mil_CamClientRect);
							bBeginDefiningRefMark = TRUE;
							SendMessage(hUserMsg, WM_SETTEXT, NULL, (LPARAM)L"Click and drag to define rectangle around the registration mark.");
							RedrawWindow(hWindow, NULL, NULL, RDW_INVALIDATE);
							SetCapture(hWindow);
						}

					}else{  // Else (mil_hdcMem == NULL)
						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"CreateCompatibleDC() Failed!\n") > 0))
							OutputDebugString(mil_DbgMesg);
					}
					EndPaint(mli_CameraHwnd, &ps);
				} // End if (hdc != NULL)
				break;
			} // End case 3:
		} // End switch (wmId)
		break;
	} // End case WM_COMMAND:
	break;

	case WM_PAINT:
	{
		PRECT prect;
		HDC hdc;
		HDC hdc_cam;
		PAINTSTRUCT ps;
		PAINTSTRUCT ps2;
		hdc = BeginPaint(hWindow, &ps);
		if (hbmp_image0Bmp != NULL)
		{
			hdc_cam = BeginPaint(mli_CameraHwnd, &ps2);

			BITMAP bm;
			// Find out what size the input image is
			GetObject(hbmp_image0Bmp, sizeof(BITMAP), &bm);
			// Set stretch mode as COLORONCOLOR and then 
			// copy from the source bitmap into the display window
			SetStretchBltMode(hdc_cam, COLORONCOLOR);
			StretchBlt(hdc_cam, 0, 0, mli_CamWindowX_Dim, mli_CamWindowY_Dim, mil_hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
			// Were're done
			// Use mil_hdcMem as the Src to restore the current displayed image with,
			// * Don't: DeleteDC(mil_hdcMem) and and DeleteObject(hbmp_image0Bmp);
			// ... until later ...
			// mil_hdcMem = NULL;
			// hbmp_image0Bmp = NULL;
			HBRUSH hbr = NULL;
			if (!bDrawFoundRefMark)
			{
				hbr = CreateSolidBrush(RGB(0, 200, 0));
				for (int iIndex = 0; iIndex < mil_RectsToDraw; iIndex++)
					FrameRect(hdc_cam, &mil_RectsDefined[iIndex], hbr);
			}else{
				if (!mil_bRefMarksDrawn)
				{
					hbr = CreateSolidBrush(RGB(200, 0, 0));

					if ((mil_RectsToDraw > 1) && (mil_ToDraw == 0))
					{
						mil_ToDraw = LocateRefMarks(&mil_RectsDefined[REF_RECTANGLE], hdc_cam);
						bRefMarkSearched = (mil_ToDraw > 0);
					}

					int ZoneWidth = (mil_RectsDefined[ZONE_RECTANGLE].right - mil_RectsDefined[ZONE_RECTANGLE].left);
					int ZoneHeight = (mil_RectsDefined[ZONE_RECTANGLE].bottom - mil_RectsDefined[ZONE_RECTANGLE].top);

					for (int RectIndex = 0; RectIndex < mil_ToDraw; RectIndex++)
					{
						// Draw a frame around the ref mark
						FrameRect(hdc_cam, &mil_RefMarkRects[RectIndex], hbr);
						

						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"RefMark Rect[%02d] - Left: %04d  Top: %04d Right: %04d Bottom: %04d\n", RectIndex, mil_RefMarkRects[RectIndex].left, mil_RefMarkRects[RectIndex].top, mil_RefMarkRects[RectIndex].right, mil_RefMarkRects[RectIndex].bottom) > 0))
							OutputDebugString(mil_DbgMesg);

						// Convert the rectangle to the inspection zone
						// ( Clamp the values to screen image dimensions )
						mil_RefMarkRects[RectIndex].right = min((int)(mil_RefMarkRects[RectIndex].left + ZoneWidth), (int)(CANERA_WIDTH * mil_Dim_Multiplier));
						mil_RefMarkRects[RectIndex].bottom = min((int)(mil_RefMarkRects[RectIndex].top + ZoneHeight), (int)(CANERA_HEIGHT * mil_Dim_Multiplier));

						// Draw a frame around the inspection zone
						FrameRect(hdc_cam, &mil_RefMarkRects[RectIndex], hbr);

						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Inspect zone (draw) Rect[%02d] - Left: %04d  Top: %04d Right: %04d Bottom: %04d\n", RectIndex, mil_RefMarkRects[RectIndex].left, mil_RefMarkRects[RectIndex].top, mil_RefMarkRects[RectIndex].right, mil_RefMarkRects[RectIndex].bottom) > 0))
							OutputDebugString(mil_DbgMesg);

						// Convert the rect to full size image dimensions
						mil_RefMarkRects[RectIndex].left = (mil_RefMarkRects[RectIndex].left * mil_Dim_Conversion);
						mil_RefMarkRects[RectIndex].right = (mil_RefMarkRects[RectIndex].right * mil_Dim_Conversion);
						mil_RefMarkRects[RectIndex].top = (mil_RefMarkRects[RectIndex].top * mil_Dim_Conversion);
						mil_RefMarkRects[RectIndex].bottom = (mil_RefMarkRects[RectIndex].bottom * mil_Dim_Conversion);

						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Inspect zone (full) Rect[%02d] - Left: %04d  Top: %04d Right: %04d Bottom: %04d\n", RectIndex, mil_RefMarkRects[RectIndex].left, mil_RefMarkRects[RectIndex].top, mil_RefMarkRects[RectIndex].right, mil_RefMarkRects[RectIndex].bottom) > 0))
							OutputDebugString(mil_DbgMesg);

					}

					int FullZoneWidth;
					int FullZoneHeight;

					if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Found %02d RefMarks\n", mil_ToDraw) > 0))
					SendMessage(hUserMsg, WM_SETTEXT, NULL, (LPARAM)mil_DbgMesg);

					// Copy Zone Zero Rect to the "Golden Image"
					FullZoneWidth = (mil_RefMarkRects[0].right - mil_RefMarkRects[0].left);
					FullZoneHeight = (mil_RefMarkRects[0].bottom - mil_RefMarkRects[0].top);
					mil_RefImage = (RGBTRIPLE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ((sizeof(RGBTRIPLE) * (FullZoneWidth * FullZoneHeight))));

					InitializeGoldenImage(mil_RefImage, (RGBTRIPLE *) mil_pImageBits, FullZoneWidth, FullZoneHeight);
					CreateReferenceImage((RGBTRIPLE *) mil_pImageBits, FullZoneWidth, FullZoneHeight);

					mil_ProcessImage = new ProcessImage((byte *)mil_RefImage, FullZoneWidth, FullZoneHeight, (size_t)FullZoneWidth);
					mil_bRefMarksDrawn = true;
				}
			}

			if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"WM_PAINT %d Rects drawn\n", mil_RectsToDraw) > 0))
				OutputDebugString(mil_DbgMesg);
			if(hbr != NULL)
				DeleteObject(hbr);

			EndPaint(mli_CameraHwnd, &ps2);
			// Spin Off thread to do Setup
			/* HANDLE WINAPI CreateThread(
                             _In_opt_  LPSECURITY_ATTRIBUTES  lpThreadAttributes,
                             _In_      SIZE_T                 dwStackSize,
                             _In_      LPTHREAD_START_ROUTINE lpStartAddress,
                             _In_opt_  LPVOID                 lpParameter,
                             _In_      DWORD                  dwCreationFlags,
                             _Out_opt_ LPDWORD                lpThreadId
                             );*/
			// mil_hSetupThread = CreateThread(NULL,(SIZE_T) SETUPTHREAD_STACK_SIZE,SetupThreadProc,(LPVOID) hbmp_image0Bmp,0, (LPDWORD) &mil_lpThreadId);
		}

		EndPaint(hWindow, &ps);

		break;
	}

	case WM_DESTROY:
		if (mil_hdcMem != NULL)
			DeleteDC(mil_hdcMem);
		if (hbmp_image0Bmp != NULL)
			DeleteObject(hbmp_image0Bmp);
		if(mil_pImageBits != NULL)
			GlobalFree((HGLOBAL)mil_pImageBits);
		if(mil_RefMarkRects != NULL)
			HeapFree(GetProcessHeap(), 0, mil_RefMarkRects);
		ReleaseCapture();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWindow, WndMessage, wParam, lParam);
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

bool InitializeGoldenImage(RGBTRIPLE *GoldImage, RGBTRIPLE *FirstImage, UINT FullZoneWidth, UINT FullZoneHeight )
{
	if (mil_RefImage != NULL)
	{

		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"GoldImg - ImageStart: 0x%08X  ImageEnd: 0x%08X\n", (UINT)(&mil_RefImage[0]), (UINT)(&mil_RefImage[(FullZoneWidth * FullZoneHeight)])) > 0))
			OutputDebugString(mil_DbgMesg);

		//  Copy the image Row x Row
		for (int rowIndex = 0; rowIndex < FullZoneHeight; rowIndex++)
		{
			// Note that the full size image is flipped around the X axis in memory meaning that Row zero,
			// Column zero is located at: (RGBTRIPLE *)mil_pImageBits[(CANERA_WIDTH * CANERA_HEIGHT)].
			// The "Golden Image" is not flipped so Row zero, Column zero is: mil_RefImage[0]
			// This means the Row addresses (Y values), decrement for the "next" line of the SubImage, while Row
			// addresses for the Golden Image increment. Column addresses (X values) increment for both images.

			// Point to the beginning of the current row
			mil_SubImage = (RGBTRIPLE *)mil_pImageBits;
			// SubImage += (CANERA_WIDTH * CANERA_HEIGHT);
			mil_SubImage += (CANERA_WIDTH * (CANERA_HEIGHT - 1));
			mil_SubImage -= (mil_RefMarkRects[0].top * CANERA_WIDTH);
			mil_SubImage += mil_RefMarkRects[0].left;
			mil_SubImage -= (rowIndex * CANERA_WIDTH);

			if (rowIndex == 0)
			{
				RGBTRIPLE * EndAddress = (RGBTRIPLE *)mil_pImageBits;
				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"SubImage - ImageStart: 0x%08X  ImageEnd: 0x%08X\n", (UINT)(&EndAddress[((mil_RefMarkRects[0].top * CANERA_WIDTH) + mil_RefMarkRects[0].left)]), (UINT)(&EndAddress[((mil_RefMarkRects[0].bottom * CANERA_WIDTH) + mil_RefMarkRects[0].right)])) > 0))
					OutputDebugString(mil_DbgMesg);

				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Inspect zone Zero Rect - Left: %4d  Top: %04d Right: %04d Bottom: %04d\n", mil_RefMarkRects[0].left, mil_RefMarkRects[0].top, mil_RefMarkRects[0].right, mil_RefMarkRects[0].bottom) > 0))
					OutputDebugString(mil_DbgMesg);

			}

			for (int columnIndex = 0; columnIndex < FullZoneWidth; columnIndex++)
				*GoldImage++ = *mil_SubImage++;

		}

	}

}

// }


bool CreateReferenceImage(RGBTRIPLE *GoldImage, RGBTRIPLE *FullImage, RECT SubImageRect, UINT FullZoneWidth, UINT FullZoneHeight)
{
	
	// Loop through all the inspection zones we've found and write back the average of 
	// the individula Red Green and Blue values of the two images to the "Golden Image."
	
	RGBTRIPLE * EndAddress;
	RGBTRIPLE *FullEndAddress;
	RGBTRIPLE *FullStartAddress;
	RGBTRIPLE *SubEndAddress;
	RGBTRIPLE *SubStartAddress;
	RGBTRIPLE *GoldEndAddress = &mil_RefImage[FullZoneWidth * FullZoneHeight];
	RGBTRIPLE *GoldStartAddress = mil_RefImage;

	// Loop through all the full and partial inspection zones we've found
	for (int RectIndex = 1; RectIndex < mil_ToDraw; RectIndex++)
	{
		RGBTRIPLE *GoldImage;
		// RGBTRIPLE *FullImage = (RGBTRIPLE *)mil_pImageBits;
		
		UINT SubImageHeight = (mil_RefMarkRects[RectIndex].bottom - mil_RefMarkRects[RectIndex].top);
		UINT SubImageWidth = (mil_RefMarkRects[RectIndex].right - mil_RefMarkRects[RectIndex].left);

		// Deal with the image row x row first
		for (int rowIndex = 0; rowIndex < (SubImageHeight - 1); rowIndex++)
		{
			RGBTRIPLE *Dest = GoldImage;
			// Init pointer to Ref Image
			Dest = GoldImage;					// Point to beginning
			Dest += (rowIndex * FullZoneWidth);	// Increment to current row

			RGBTRIPLE *SubImage;
			SubImage = (RGBTRIPLE *)FullImage;
			// The image is flipped around the X axis in memory (but Not the Y axis). 
			// so the "last line" in the negative domain is camera height - 1
			SubImage += (CANERA_WIDTH * (CANERA_HEIGHT - 1));				// Point to last line
			SubImage -= (mil_RefMarkRects[RectIndex].top * CANERA_WIDTH);	// Decrement to the top of the sub-image
			SubImage += mil_RefMarkRects[RectIndex].left;					// Move pointer to column zero
			SubImage -= (rowIndex * CANERA_WIDTH);							// Decrement to the current line


			RGBTRIPLE *EndAddress = (RGBTRIPLE *)mil_pImageBits;
			RGBTRIPLE *FullEndAddress = &EndAddress[(CANERA_WIDTH * CANERA_HEIGHT)]; //  Note We're pointing to the first address _after_ the last valid pixel
			// The above is is the first addresss past the last valid pixel in the image 
			// [-*- You can't increment this address in the X direction and write to it ! -*-]
			RGBTRIPLE *FullStartAddress = (RGBTRIPLE *)mil_pImageBits;
			int RectIndex = 0;
			// These are just diagnostic variables for diagnosing boundry conditions and access violations [Delete From Release ver]
			RGBTRIPLE *SubEndAddress = &EndAddress[((mil_RefMarkRects[RectIndex].bottom * CANERA_WIDTH) + mil_RefMarkRects[RectIndex].right)];		// WRT - Top and bottom start and end are somewhat "subjective" but the
			RGBTRIPLE *SubStartAddress = &EndAddress[((mil_RefMarkRects[RectIndex].top * CANERA_WIDTH) + mil_RefMarkRects[RectIndex].left)];		// important thing is we are noting the correct addresses involved


			for(int columnIndex = 0; columnIndex < (SubImageWidth); columnIndex++)
			{
				// Print diagnostic message if we're at the beginning of a line
				if ((rowIndex == 0) && (columnIndex == 0))
				{

					if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"FullImage - ImageStart: 0x%08X  ImageEnd: 0x%08X\n", (UINT)(FullStartAddress), (UINT)(FullEndAddress)) > 0))
						OutputDebugString(mil_DbgMesg);

					if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"SubImage - ImageStart: 0x%08X  ImageEnd: 0x%08X\n", (UINT)(SubStartAddress), (UINT)(SubEndAddress)) > 0))
						OutputDebugString(mil_DbgMesg);

					if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"SubImagCalc - ImageStart: 0x%08X  ImageEnd: 0x%08X\n", (UINT)(SubImage), (UINT)(&SubImage[SubImageWidth])) > 0))
						OutputDebugString(mil_DbgMesg);

					if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Inspect zone[%02d]  Rect - Left: %04d  Top: %04d Right: %04d Bottom: %04d\n", RectIndex, mil_RefMarkRects[RectIndex].left, mil_RefMarkRects[RectIndex].top, mil_RefMarkRects[RectIndex].right, mil_RefMarkRects[RectIndex].bottom) > 0))
						OutputDebugString(mil_DbgMesg);

				}


				Dest->rgbtRed = ((GoldImage->rgbtRed + SubImage->rgbtRed) >> 1);
				Dest->rgbtGreen = ((GoldImage->rgbtGreen + SubImage->rgbtGreen) >> 1);
				Dest->rgbtBlue = ((GoldImage->rgbtBlue + SubImage->rgbtBlue) >> 1);
				Dest++;
				SubImage++;

				// if( (SubImage < FullStartAddress) || (SubImage < SubStartAddress) || (SubImage > FullEndAddress) || (SubImage > SubEndAddress) )
				if (SubImage < FullStartAddress)
					SubImage++;

				if (SubImage > FullEndAddress)
					SubImage--;

				if (GoldImage < GoldStartAddress)
					GoldImage++;

				if (GoldImage > GoldEndAddress)
					GoldImage--;


			}
		}
	}

}

bool CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{

	HANDLE hf;                  // file handle
	BITMAPFILEHEADER hdr;       // bitmap file-header
	PBITMAPINFOHEADER pbih;     // bitmap info-header
	
	DWORD dwTotal;              // total count of bytes
	DWORD cb;                   // incremental count of bytes
	BYTE *hp;                   // byte pointer
	DWORD dwTmp;
	bool bErrorOccured = false;

	pbih = (PBITMAPINFOHEADER)pbi;
	mil_pImageBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!mil_pImageBits)
	{
		MessageBox(hwnd, L"GlobalAlloc", L"Error", MB_OK);
		bErrorOccured = true;
	}

	// Retrieve the color table (RGBTRIPLE array) and the bits
	// (array of palette indices) from the DIB.
	if (!GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, mil_pImageBits, pbi, DIB_RGB_COLORS))
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
	hp = mil_pImageBits;

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
	
	// mil_ProcessImage = new ProcessImage(pszFile);
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
		if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Avialable resolutions for: %ls \n", mil_CameraNames[iIndex]) >= 1))
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
						mil_CameraNames[NumCamerasFound - 1] = new wchar_t[MAX_LOADSTRING];
						wcsncpy_s(mil_CameraNames[NumCamerasFound - 1], MAX_LOADSTRING, varName.bstrVal, _TRUNCATE);
						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Found Camera: %ls\n", mil_CameraNames[NumCamerasFound - 1]) >= 1))
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

// There is a new image to process, we're creating/adding to our initialization info 
// run: 

DWORD WINAPI SetupThreadProc(_In_ LPVOID lpParameter)
{
	DWORD dwResult = 0;
	HBITMAP     hbmp_image = (HBITMAP)lpParameter;

	if (hbmp_image == NULL)
		return 1;

	Button_Enable(mil_hButtonRefImage,FALSE);

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"Disabled Creat Ref Image Button\n") >= 1))
		OutputDebugString(mil_DbgMesg);

	return dwResult;
}

BOOL bPointIsInRect(POINT Candidate, RECT Area)
{

	if ((Candidate.x < Area.left) || (Candidate.x > Area.right))
		return false;
	if ((Candidate.y < Area.top) || (Candidate.y > Area.bottom))
		return false;
	return true;



}


BOOL GetClientScreenCoordinates(HWND hWindowParent, HWND hWindowChild, RECT *pRect)
{

	POINT ptClientChildUL; // client area upper left corner  
	POINT ptClientChildLR; // client area lower right corner  
	POINT ptClientParentUL; // client area upper left corner  
	POINT ptClientParentLR; // client area lower right corner  
	RECT rcClientParent; // client area rectangle of parent  
	RECT rcClientChild; // client area rectangle of parent  

	if ((hWindowParent == NULL) || (hWindowChild == NULL) || (pRect == NULL))
		return FALSE;
	// Convert the client coordinates of the client area  
	// rectangle to screen coordinates and save them in a  
	// rectangle. The rectangle is passed to the ClipCursor  
	// function during WM_LBUTTONDOWN processing.  
	GetClientRect(hWindowParent, &rcClientParent);
	GetClientRect(hWindowChild, &rcClientChild);

	ptClientParentUL.x = rcClientParent.left;
	ptClientParentUL.y = rcClientParent.top;
	ptClientParentLR.x = rcClientParent.right;
	ptClientParentLR.y = rcClientParent.bottom;

	ptClientChildUL.x = rcClientChild.left;
	ptClientChildUL.y = rcClientChild.top;
	ptClientChildLR.x = rcClientChild.right;
	ptClientChildLR.y = rcClientChild.bottom;

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"GCSC Client Rect Parent - Left: %04d Top: %04d Right: %04d Buttom: %04d\n", ptClientParentUL.x, ptClientParentUL.y, ptClientParentLR.x, ptClientParentLR.y) > 0))
		OutputDebugString(mil_DbgMesg);

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"GCSC Client Rect Child - Left: %04d Top: %04d Right: %04d Buttom: %04d\n", ptClientChildUL.x, ptClientChildUL.y, ptClientChildLR.x, ptClientChildLR.y) > 0))
		OutputDebugString(mil_DbgMesg);

	ClientToScreen(hWindowParent, &ptClientParentUL);
	ClientToScreen(hWindowParent, &ptClientParentLR);

	ClientToScreen(hWindowChild, &ptClientChildUL);
	ClientToScreen(hWindowChild, &ptClientChildLR);

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"GCSC Screen Rect Parent - Left: %04d Top: %04d Right: %04d Buttom: %04d\n", ptClientParentUL.x, ptClientParentUL.y, ptClientParentLR.x, ptClientParentLR.y) > 0))
	OutputDebugString(mil_DbgMesg);

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"GCSC Screen Rect Child - Left: %04d Top: %04d Right: %04d Buttom: %04d\n", ptClientChildUL.x, ptClientChildUL.y, ptClientChildLR.x, ptClientChildLR.y) > 0))
		OutputDebugString(mil_DbgMesg);

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"GCSC Parrent Rel Child Rect - Left: %04d Top: %04d Right: %04d Buttom: %04d\n", (ptClientChildUL.x - ptClientParentUL.x), (ptClientChildUL.y - ptClientParentUL.y), (ptClientChildLR.x - ptClientParentUL.x), (ptClientChildLR.y - ptClientParentUL.y)) > 0))
		OutputDebugString(mil_DbgMesg);

	// return(SetRect(pRect, ptClientChildUL.x, ptClientChildUL.y, ptClientChildLR.x, ptClientChildLR.y));
	return(SetRect(pRect, (ptClientChildUL.x - ptClientParentUL.x), (ptClientChildUL.y - ptClientParentUL.y), (ptClientChildLR.x - ptClientParentUL.x), (ptClientChildLR.y - ptClientParentUL.y)));
}

void Screen2Client(POINT *MousePosition)
{

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"S2C Point IN - x: %04d y: %04d \n", MousePosition->x, MousePosition->y) > 0))
		OutputDebugString(mil_DbgMesg);

	MousePosition->x -= mil_CamClientRect.left;
	MousePosition->x += mil_MainClientRect.left;
	MousePosition->y -= mil_CamClientRect.top;
	MousePosition->y += mil_MainClientRect.top;

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"S2C Point OUT - x: %04d y: %04d \n", MousePosition->x, MousePosition->y) > 0))
		OutputDebugString(mil_DbgMesg);

}

UINT LocateRefMarks(RECT *CaptureRect, HDC RMarkHandel)
{
	UINT RefMarksFound = 0;
	UINT ColumIndex = 0;
	UINT RowIndex = 0;
	RGBTRIPLE *rgbColumnValues = NULL;
	RGBTRIPLE *rgbRowValues = NULL;
	UINT NumberOfColumnValues;
	UINT NumberOfRowValues;
	RGBTRIPLE *rgbColumnStart;
	RGBTRIPLE *rgbRowStart;
	RGBTRIPLE *rgbCurrentColumPixel;
	RGBTRIPLE *rgbCurrentRowPixel;
	UINT ImageWidth;
	UINT ImageHeight;
	UINT RefMarkColumnCount = 0;
	UINT RefMarkRowCount = 0;
	UINT *BeginningEndingRows = NULL;
	UINT *BeginningEndingColumns = NULL;
	UINT *RowsFound = NULL;
	UINT *ColumnsFound = NULL;

	if(CaptureRect == NULL) 
		return RefMarksFound;

	// The image on screen may be smaller than the full size
	// image map the screen cordinates to the full size dimensions
	UINT refMarkTop = (CaptureRect->top * mil_Dim_Conversion);
	UINT refMarkLeft = (CaptureRect->left * mil_Dim_Conversion);
	UINT refMarkBottom = (CaptureRect->bottom * mil_Dim_Conversion);
	UINT refMarkRight = (CaptureRect->right * mil_Dim_Conversion);

	NumberOfColumnValues = ((refMarkBottom) - (refMarkTop));
	NumberOfRowValues = ((refMarkRight) - (refMarkLeft));
	ImageWidth = mil_pImageBitMapInfo->bmiHeader.biWidth;
	ImageHeight = mil_pImageBitMapInfo->bmiHeader.biHeight;

	if ((NumberOfColumnValues == 0) || (NumberOfRowValues == 0))
		return 0;

	rgbColumnValues = (RGBTRIPLE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (sizeof(RGBTRIPLE) * NumberOfColumnValues) );
	rgbRowValues = (RGBTRIPLE *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (sizeof(RGBTRIPLE) * NumberOfRowValues));

	RowsFound = BeginningEndingRows = (UINT *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ((sizeof(UINT) * (ImageHeight / NumberOfColumnValues)) + 1) );
	ColumnsFound = BeginningEndingColumns = (UINT *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ((sizeof(UINT) * (ImageWidth / NumberOfColumnValues)) + 1));

	rgbColumnStart = rgbRowStart = (RGBTRIPLE *) mil_pImageBits;

	if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"LocRef Column - ImageStart: 0x%08X  ImageEnd: 0x%08X\n", (UINT)(mil_pImageBits), (UINT) (&rgbColumnStart[(ImageWidth * ImageHeight)])) > 0))
		OutputDebugString(mil_DbgMesg);

	rgbColumnStart += ((ImageWidth) * (refMarkTop)) + (refMarkLeft) + (NumberOfRowValues >> 1);
	rgbRowStart += (((ImageWidth) * ((refMarkTop) + (NumberOfColumnValues >> 1))) + (refMarkLeft) );

	// Read the center column from the refmark rectangle
	for (int iIndex = 0; iIndex < NumberOfColumnValues; iIndex++)
		rgbColumnValues[iIndex] = rgbColumnStart[iIndex * ImageWidth];
	// Read the center row from the refmark rectangle
	for (int iIndex = 0; iIndex < NumberOfRowValues; iIndex++)
		rgbRowValues[iIndex] = rgbRowStart[iIndex];

	// Itterate over each column of pixels that could possibly contain a matching sequence to the reference pixels
	for (int CurrentColumnIndex = 0; CurrentColumnIndex < (ImageWidth - (NumberOfRowValues >> 1)); CurrentColumnIndex++)
	{
		// Point to the beginning pixel of the current column
		rgbCurrentColumPixel = (RGBTRIPLE *) mil_pImageBits;
		rgbCurrentColumPixel += CurrentColumnIndex;
		// Beging checking each pixel in the current column
		for (int CurrentColumnRow = 0; CurrentColumnRow < (ImageHeight - NumberOfColumnValues); CurrentColumnRow++)
		{
			// If the value of all the current sequence of pixels matches the refpixels within the tolerance value
			if (isMatchToRefColumn(rgbColumnValues, rgbCurrentColumPixel, ImageWidth, NumberOfColumnValues, (BYTE) 3) )
			{
				RefMarkColumnCount++;

				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"LocRef Column Found ! - Column Index: %04d  Colum Row: %04d  Address: 0x%08X\n", CurrentColumnIndex, CurrentColumnRow, rgbCurrentColumPixel) > 0))
					OutputDebugString(mil_DbgMesg);

				// Record this as the center column of a ref mark
				*BeginningEndingRows = (CurrentColumnRow * mil_Dim_Multiplier);
				BeginningEndingRows++;
				*BeginningEndingRows = ((CurrentColumnRow + NumberOfColumnValues) * mil_Dim_Multiplier);
				BeginningEndingRows++;
                // Jump forward a refmark height in pixels
				CurrentColumnRow += NumberOfColumnValues;
				rgbCurrentColumPixel += (ImageWidth * NumberOfColumnValues);
			}else{ // Else no match inc to next pixel
				rgbCurrentColumPixel += ImageWidth;
			}
		}
	}

	// Itterate over each row of pixels that may possibly contain a matching sequence to the reference pixels
	for (int CurrentRowIndex = 0; CurrentRowIndex < (ImageHeight - (NumberOfColumnValues >> 1)); CurrentRowIndex++)
	{
		// Point to the beginning pixel of the current row
		rgbCurrentRowPixel = (RGBTRIPLE *) mil_pImageBits;
		rgbCurrentRowPixel += (CurrentRowIndex * ImageWidth);
		// Itterate over the columns in the current row looking for a matching sequence of pixels
		for (int CurrentRowColumn = 0; CurrentRowColumn < (ImageWidth - NumberOfRowValues); CurrentRowColumn++)
		{
			// If all the pixels in rgbRowValues match the current pixels in the image
			if (isMatchToRefRow(rgbRowValues, rgbCurrentRowPixel, NumberOfRowValues, (BYTE) 3))
			{
				RefMarkRowCount++;

				if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"LocRef Row Found ! - Column Index: %04d  Colum Row: %04d  Address: 0x%08X\n", CurrentRowColumn, CurrentRowIndex, rgbCurrentRowPixel) > 0))
					OutputDebugString(mil_DbgMesg);

				// Record this as the left and right side of a reference rectangle 
				*BeginningEndingColumns = (CurrentRowColumn * mil_Dim_Multiplier);
				BeginningEndingColumns++;
				*BeginningEndingColumns = ((CurrentRowColumn + NumberOfRowValues) * mil_Dim_Multiplier);
				BeginningEndingColumns++;
				// Jump forward a refmark width
				CurrentRowColumn += NumberOfRowValues;
				rgbCurrentRowPixel += NumberOfRowValues;
			}else{ // Else no match inc to next pixel
				rgbCurrentRowPixel++;
			}
		}
	}

	// If we found at least one refmark
	if ((RefMarkRowCount > 0) && (RefMarkColumnCount > 0))
	{
		// Reset the pointers to the beginning
		BeginningEndingColumns -= (RefMarkRowCount << 1);
		BeginningEndingRows -= (RefMarkColumnCount << 1);
		// Allocate space to store the rects
		mil_RefMarkRects = (RECT *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ((sizeof(RECT) * (RefMarkRowCount * RefMarkColumnCount))));
		RECT CandidateRect;
		if(mil_RefMarkRects != NULL)
		{
			for (int iColumnIndex = 0; iColumnIndex < RefMarkColumnCount; iColumnIndex += 2)
			{
				for (int iRowIndex = 0; iRowIndex < RefMarkRowCount; iRowIndex += 2)
				{
					CandidateRect.left = BeginningEndingColumns[iColumnIndex];
					CandidateRect.right = BeginningEndingColumns[iColumnIndex + 1];
					CandidateRect.top = BeginningEndingRows[iRowIndex];
					CandidateRect.bottom = BeginningEndingRows[iRowIndex + 1];

					if (!rectIsDuplicate(mil_RefMarkRects, CandidateRect, RefMarksFound))
					{
						mil_RefMarkRects[RefMarksFound].left = CandidateRect.left;
						mil_RefMarkRects[RefMarksFound].right = CandidateRect.right;
						mil_RefMarkRects[RefMarksFound].top = CandidateRect.top;
						mil_RefMarkRects[RefMarksFound].bottom = CandidateRect.bottom;
						RefMarksFound++;

						if ((swprintf_s(mil_DbgMesg, (size_t)MAX_PATH, L"RefMark[%02d] - Left: %04d Top: %04d Right: %04d Buttom: %04d\n", RefMarksFound, CandidateRect.left, CandidateRect.top, CandidateRect.right, CandidateRect.bottom) > 0))
							OutputDebugString(mil_DbgMesg);

					}
				}
			}
		}
	}

	if (rgbColumnValues != NULL)
		HeapFree(GetProcessHeap(), 0, rgbColumnValues);
	if (rgbRowValues != NULL)
		HeapFree(GetProcessHeap(), 0, rgbRowValues);
	if (BeginningEndingRows != NULL)
		HeapFree(GetProcessHeap(), 0, BeginningEndingRows);
	if (BeginningEndingColumns != NULL)
		HeapFree(GetProcessHeap(), 0, BeginningEndingColumns);

	return RefMarksFound;
}

BOOL isMatchToRefColumn(RGBTRIPLE *RefValues, RGBTRIPLE *CandidatePixel, UINT Increment, UINT NumberOfValues, BYTE rgbTolerance)
{
	RGBTRIPLE RefPixel;
	RGBTRIPLE ImagePixel;

	for (UINT PixelIndex = 0; PixelIndex < NumberOfValues; PixelIndex++)
	{
		RefPixel = RefValues[PixelIndex];
		ImagePixel = CandidatePixel[(PixelIndex * Increment)];
		if ((abs(RefPixel.rgbtRed - ImagePixel.rgbtRed)) > rgbTolerance)
			return false;
		if ((abs(RefPixel.rgbtGreen - ImagePixel.rgbtGreen)) > rgbTolerance)
			return false;
		if ((abs(RefPixel.rgbtBlue - ImagePixel.rgbtBlue)) > rgbTolerance)
			return false;
	}
	return true;
}

BOOL isMatchToRefRow(RGBTRIPLE *RefValues, RGBTRIPLE *CandidatePixel, UINT NumberOfValues, BYTE rgbTolerance)
{
	RGBTRIPLE RefPixel;
	RGBTRIPLE ImagePixel;

	for (UINT PixelIndex = 0; PixelIndex < NumberOfValues; PixelIndex++)
	{
		RefPixel = RefValues[PixelIndex];
		ImagePixel = CandidatePixel[PixelIndex];
		if ((abs(RefPixel.rgbtRed - ImagePixel.rgbtRed)) > rgbTolerance)
			return false;
		if ((abs(RefPixel.rgbtGreen - ImagePixel.rgbtGreen)) > rgbTolerance)
			return false;
		if ((abs(RefPixel.rgbtBlue - ImagePixel.rgbtBlue)) > rgbTolerance)
			return false;
	}
	return true;
}

BOOL rectIsDuplicate(RECT *RefMarkRects, RECT Candidate, UINT MarksFound)
{
	if( (RefMarkRects == NULL) || (MarksFound == 0) )
		return false;

	for (int index = 0; index < MarksFound ; index++)
		if((RefMarkRects[index].left == Candidate.left) && (RefMarkRects[index].right == Candidate.right) && (RefMarkRects[index].top == Candidate.top) && (RefMarkRects[index].bottom == Candidate.bottom) )
			return TRUE;

	return false;
	
}

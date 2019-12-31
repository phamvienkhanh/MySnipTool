// #define WIN32_LEAN_AND_MEAN

// #define WINVER 0x0501
// #define _WIN32_WINNT 0x0501
#define UNICODE
#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdio.h>
#include <vector>
#include "ConfigWindows.h"

#pragma comment (lib,"gdiplus.lib")
#pragma comment (lib,"gdi32.lib")
#pragma comment (lib,"ole32.lib")

using namespace Gdiplus;
using namespace std;

LPCWSTR s_szWndClassName = L"MyCapture";
LPCWSTR s_szFile = L"sunset.bmp";


bool				s_isBeginDrawRect		= false;
POINT				s_beginDrawPoint{ 0,0 };
POINT				s_endDrawPoint{ 0,0 };
//Graphics*			s_currentDrawRect	= nullptr;
//HDC					s_currentHDC;
int					s_currentCountSnap	= 0;
bool				s_toggleShowCongigWnd = true;
ConfigWindows		s_hwndCofig;
std::vector<Rect>	s_listRect;
//HBITMAP s_hBitmap = NULL;

ATOM RegisterWndClass(HINSTANCE, LPCWSTR);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//HBITMAP LoadPicture(LPCWSTR);
//BOOL DrawPicture(HDC, LPRECT);

void 	screenshot(POINT a, POINT b);
int 	GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void	DrawRect(HWND _hwnd,Rect _rect);
void	ReDrawRect(HWND _hwnd, Rect _rect);

void __InitGDI()
{
	CoInitialize(NULL);
	// Initialize GDI+.
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void __ShutdownGDI()
{
	//Gdiplus::GdiplusShutdown(gdiplusToken);
   // CoUninitialize();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevIns, LPSTR lpszArgument, int nCmdShow)
{
	HWND hWnd;
	MSG msg;

	//s_hBitmap = LoadPicture(L"sunset.bmp"); 

	RegisterWndClass(hInstance, s_szWndClassName);

	hWnd = CreateWindowEx(WS_EX_LAYERED,
		s_szWndClassName, s_szWndClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 300, 300,
		NULL, NULL, hInstance, NULL);

	
	s_hwndCofig.InitWindows(hInstance);
	//s_hwndCofig.SetSize({ 200,70 });
	s_hwndCofig.ShowWindows(true);


	__InitGDI();

	if (hWnd != NULL)
	{
		SetLayeredWindowAttributes(hWnd, RGB(0, 0, 123),
			128, LWA_COLORKEY);

		ShowWindow(hWnd, nCmdShow);

		//s_listRect.push_back(Rect(20, 30, 200, 100));
		//s_listRect.push_back(Rect(200, 300, 200, 100));

		UpdateWindow(hWnd);

		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

ATOM RegisterWndClass(HINSTANCE hInstance, LPCWSTR lpszWndClassName)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = /* (HBRUSH)(COLOR_WINDOW+1) ;*/ (HBRUSH)(CreateSolidBrush(RGB(0, 0, 123)));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = lpszWndClassName;
	return RegisterClassEx(&wcex);
}
static HDC hdcCompat;
bool needRepaint = false;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		//clean screen
		//GetClientRect(hWnd, &rect);

		// repaint
		for (size_t i = 0; i < s_listRect.size(); i++)
		{
			ReDrawRect(hWnd, s_listRect[i]);
		}

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_SPACE:
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				wstring _saveDir;
				_saveDir = s_hwndCofig.GetSaveDir();
				GetWindowRect(hWnd, &rect);
				POINT _offsetTop{ 8,31 };
				POINT _offsetBot{ 9,7 };
				screenshot({ rect.left + _offsetTop.x ,rect.top + _offsetTop.y }, { rect.right - _offsetBot.x, rect.bottom - _offsetBot.y });
				s_currentCountSnap++;
			}
		}
		break;

		case 0x41 : // key A
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				if (s_toggleShowCongigWnd)
				{
					s_hwndCofig.ShowWithAnimation();
				}
				else
				{
					s_hwndCofig.HideWithAnimation();
				}

				s_toggleShowCongigWnd = !s_toggleShowCongigWnd;
			}
		}
		break;

		case 0x44 : // key D
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				//delete list rects
				s_listRect.clear();

				//clean screen
				GetClientRect(hWnd, &rect);
				InvalidateRect(hWnd, NULL, 1);
			}
		}
		break;

		default:
			break;
		}
	}
	break;

	case WM_LBUTTONDOWN :
	{
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		{
			s_isBeginDrawRect = true;
			GetCursorPos(&s_beginDrawPoint);
			ScreenToClient(hWnd, &s_beginDrawPoint);

			s_endDrawPoint = s_beginDrawPoint;

			//s_currentHDC		= GetDC(hWnd); //BeginPaint(hWnd, &ps);
			//s_currentDrawRect	=  new Gdiplus::Graphics(s_currentHDC);
		}
	}
	break;

	case WM_LBUTTONUP :
	{
		s_isBeginDrawRect = false;
		
		s_listRect.push_back(Rect{ s_beginDrawPoint.x,
									s_beginDrawPoint.y,
									s_endDrawPoint.x,
									s_endDrawPoint.y });

		InvalidateRect(hWnd, NULL, 1);
	}
	break;

	case WM_MOUSEMOVE :
	{
		if (s_isBeginDrawRect)
		{

			POINT _endDrawPoint;
			GetCursorPos(&_endDrawPoint);
			ScreenToClient(hWnd, &_endDrawPoint);


			DrawRect(hWnd, Rect{ s_beginDrawPoint.x,
									s_beginDrawPoint.y,
									_endDrawPoint.x,
									_endDrawPoint.y });

			s_endDrawPoint = _endDrawPoint;
		}
	}
	break;


	case WM_MOVE:
	{
		RECT _windRect;
		GetWindowRect(hWnd, &_windRect);
		s_hwndCofig.SetPos({ _windRect.right,_windRect.top });
	}
	break;

	case WM_SIZING:
	{
		RECT _windRect;
		GetWindowRect(hWnd, &_windRect);
		s_hwndCofig.SetPos({ _windRect.right,_windRect.top });
	}
	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam); 
	}
	return 0;
}

// HBITMAP LoadPicture(LPCWSTR lpszFileName)
// {
//     HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, lpszFileName,
// 	IMAGE_BITMAP, 0, 0, 				
// 	LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE); 
//     return hBitmap;
// }

// BOOL DrawPicture(HDC hDC, LPRECT lpRect)
// {
// 	BITMAP bitmap;
// 	HDC hMemDC;
// 	HGDIOBJ hBitmapOld;
// 	if (s_hBitmap == NULL)
// 		return FALSE;
// 	GetObject(s_hBitmap, sizeof(BITMAP), &bitmap);
// 	hMemDC = CreateCompatibleDC(hDC);
// 	hBitmapOld = SelectObject(hMemDC, s_hBitmap);
// 	StretchBlt(hDC, 
// 		lpRect->left, lpRect->top, 
// 		lpRect->right - lpRect->left,
// 		lpRect->bottom - lpRect->top, 
// 		hMemDC, 
// 		0, 0, bitmap.bmWidth, bitmap. bmHeight, 
// 		SRCCOPY);
// 	SelectObject(hMemDC, hBitmapOld);
// 	DeleteDC(hMemDC);
// 	return TRUE;
// }

void screenshot(POINT a, POINT b)
{
	//IStream* istream = nullptr;
	// CreateStreamOnHGlobal(NULL, TRUE, &istream);

	//result = GetEncoderClsid(L"image/png", &encoderClsid);

	 // copy screen to bitmap
	HDC     hScreen = GetDC(NULL);
	HDC     hDC = CreateCompatibleDC(hScreen);
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, abs(b.x - a.x), abs(b.y - a.y));
	HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
	BOOL    bRet = BitBlt(hDC, 0, 0, abs(b.x - a.x), abs(b.y - a.y), hScreen, a.x, a.y, SRCCOPY);

	// save bitmap to clipboard
	// OpenClipboard(NULL);
	// EmptyClipboard();
	// SetClipboardData(CF_BITMAP, hBitmap);
	// CloseClipboard();

	//save png image
	Gdiplus::Bitmap bmp(hBitmap, nullptr);

	//write to IStream
	IStream* istream = nullptr;
	CreateStreamOnHGlobal(NULL, TRUE, &istream);

	CLSID clsid_png;
	GetEncoderClsid(L"image/png", &clsid_png);
	Gdiplus::Status status = bmp.Save(istream, &clsid_png, NULL);

	//get memory handle associated with istream
	HGLOBAL hg = NULL;
	GetHGlobalFromStream(istream, &hg);

	//copy IStream to buffer
	std::vector<BYTE> data;
	int bufsize = GlobalSize(hg);
	data.resize(bufsize);

	//lock & unlock memory
	LPVOID pimage = GlobalLock(hg);
	memcpy(&data[0], pimage, bufsize);
	GlobalUnlock(hg);
	istream->Release();

	//write from memory to file for testing:
	wstring _saveDir = s_hwndCofig.GetSaveDir();
	if (*(_saveDir.end() - 1) != wchar_t("\\"))
	{
		_saveDir += L"\\";
	}

	CreateDirectory(_saveDir.c_str(), NULL);

	_saveDir = _saveDir + std::to_wstring(s_currentCountSnap) + L".png";
	std::ofstream fout(_saveDir, std::ios::binary|std::ios::out);
	fout.write((char*)data.data(), data.size());

	

	// clean up
	SelectObject(hDC, old_obj);
	DeleteDC(hDC);
	ReleaseDC(NULL, hScreen);
	DeleteObject(hBitmap);
}

Rect oldRect;
void DrawRect(HWND _hwnd,Rect _rect)
{

	HDC hdc = GetDC(_hwnd);
	SetROP2(hdc, R2_NOTXORPEN);
	//SelectObject(hdc, GetStockObject(NULL)); // //GetStockObject(NULL)

	RECT __rect{ _rect.X, _rect.Y, _rect.Width, _rect.Height };

	Rectangle(hdc, oldRect.X, oldRect.Y, oldRect.Width, oldRect.Height);
	Rectangle(hdc, _rect.X, _rect.Y, _rect.Width, _rect.Height);
	oldRect = _rect;

	DeleteDC(hdc);
	ReleaseDC(_hwnd, hdc);
}

void ReDrawRect(HWND _hwnd, Rect _rect)
{

	HDC hdc = GetDC(_hwnd);
	//SelectObject(hdc, CreateSolidBrush(RGB(255, 0, 0))); // //GetStockObject(NULL)

	RECT __rect{ _rect.X, _rect.Y, _rect.Width, _rect.Height };

	FrameRect(hdc, &__rect, CreateSolidBrush(RGB(255, 0, 0)));

	DeleteDC(hdc);
	ReleaseDC(_hwnd, hdc);
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}
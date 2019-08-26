#pragma once
#include <Windows.h>
#include <string>

#define DEFAULT_WIDTH  500
#define DEFAULT_HEIGHT 30

class ConfigWindows
{
public:
	HWND		m_hwnd;
	HWND		m_hTextBox;
	LPCWSTR		m_szWndClassName;
	POINT		m_pos;
	POINT		m_size;

public:
	 void InitWindows(HINSTANCE _hInstance);
	 void ShowWindows(bool _show);
	 void SetPos(POINT _pos);
	 void SetSize(POINT _size);

	 void ShowWithAnimation();
	 void HideWithAnimation();

	 std::wstring GetSaveDir();

public:
	ConfigWindows();
	~ConfigWindows();
};


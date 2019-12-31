#include "ConfigWindows.h"

#define EDIT_CMD 0

extern ConfigWindows s_hwndCofig;

LRESULT CALLBACK ConfigWndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		//DrawPicture(hdc, &rect);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case EDIT_CMD:
		{
			
		}
		break;

		default:
			break;
		}
	}
	break;

	/*case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case 0x41:
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
				s_hwndCofig.HideWithAnimation();
			}
		}
		break;

		default:
			break;
		}
	}
	break;*/

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


ATOM configWRegisterWndClass(HINSTANCE hInstance, LPCWSTR lpszWndClassName)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ConfigWndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = /* (HBRUSH)(COLOR_WINDOW+1) ;*/ (HBRUSH)(CreateSolidBrush(RGB(0, 0, 123)));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = lpszWndClassName;
	return RegisterClassEx(&wcex);
}

void ConfigWindows::InitWindows(HINSTANCE _hInstance)
{

	configWRegisterWndClass(_hInstance, m_szWndClassName);

	m_hwnd		= CreateWindowEx(WS_EX_NOACTIVATE, m_szWndClassName, m_szWndClassName, WS_POPUP, 0, 0, 0, 0, NULL, NULL, _hInstance,NULL);
	m_hTextBox = CreateWindow(L"EDIT", L"", WS_CHILD, 3, 3, 494, 24, m_hwnd, (HMENU)EDIT_CMD, _hInstance, NULL);
}

void ConfigWindows::ShowWindows(bool _show)
{
	ShowWindow(m_hwnd, _show);
	UpdateWindow(m_hwnd);

	ShowWindow(m_hTextBox, _show);
	UpdateWindow(m_hTextBox);
}

void ConfigWindows::SetPos(POINT _pos)
{
	SetWindowPos(m_hwnd, HWND_TOP, _pos.x, _pos.y, 0, 0, SWP_NOSIZE);
	m_pos = _pos;
}

void ConfigWindows::SetSize(POINT _size)
{
	SetWindowPos(m_hwnd, HWND_TOP, 0, 0, _size.x, _size.y, SWP_NOMOVE);
	m_size = _size;
}

void ConfigWindows::ShowWithAnimation()
{
	while (m_size.x <  DEFAULT_WIDTH)
	{
		m_size.x += 2;
		SetSize({ m_size.x,m_size.y });
		Sleep(1);
	}
}

void ConfigWindows::HideWithAnimation()
{
	while (m_size.x > 0)
	{
		m_size.x -= 2;
		SetSize({ m_size.x,m_size.y });
		Sleep(1);
	}
}

std::wstring ConfigWindows::GetSaveDir()
{
	WCHAR _tmp[1024];
	GetDlgItemText(m_hwnd, EDIT_CMD, _tmp, 1024);
	return std::wstring(_tmp);
}

ConfigWindows::ConfigWindows()
{
	m_pos	= { 0,0 };
	m_size	= { 0,DEFAULT_HEIGHT };
	m_hwnd	= NULL;

	m_szWndClassName = L"configWindows";
}

ConfigWindows::~ConfigWindows()
{
}

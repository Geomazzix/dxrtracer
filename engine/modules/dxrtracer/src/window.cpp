#include "dxrtracer/window.h"
#include <core/debug.h>

namespace dxray
{
	WinApiWindow::WinApiWindow(const WindowCreationInfo& a_createInfo) :
		m_title(a_createInfo.Title),
		m_className("winApiWindow"),
		m_handle(nullptr),
		m_windowRect(a_createInfo.Rect),
		m_bIsMinimized(false),
		m_bIsMaximized(true),
		m_bIsFocused(true),
		m_bIsResizing(false),
		m_bIsFlaggedForClosing(false)
	{
		const HINSTANCE& appModule = static_cast<HINSTANCE>(GetModuleHandle(NULL));
		const WNDCLASSEX windowClass =
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = &WinApiWindow::WindowProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = appModule,
			.hIcon = LoadIcon(appModule, IDI_APPLICATION),		//101 = the default window icon
			.hCursor = LoadCursor(NULL, IDC_ARROW),
			.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
			.lpszMenuName = NULL,
			.lpszClassName = m_className.c_str(),
			.hIconSm = LoadIcon(appModule, IDI_APPLICATION)		//101 = the default window icon
		};
		DXRAY_ASSERT(RegisterClassEx(&windowClass) > 0);

		//Calculate the window size and position properties.
		RECT windowCanvas = 
		{
			.left = static_cast<LONG>(a_createInfo.Rect.x),
			.top = static_cast<LONG>(a_createInfo.Rect.y),
			.right = static_cast<LONG>(a_createInfo.Rect.Width),
			.bottom = static_cast<LONG>(a_createInfo.Rect.Height)
		};
		DXRAY_ASSERT(AdjustWindowRect(&windowCanvas, WS_OVERLAPPEDWINDOW, false) > 0);

		//Create the window.
		m_handle = CreateWindowEx(
			WS_EX_LTRREADING,
			m_className.c_str(),
			m_title.c_str(),
			WS_OVERLAPPEDWINDOW,
			vath::Max<i32>(0, (GetSystemMetrics(SM_CXSCREEN) - m_windowRect.Dimensions.x) / 2),
			vath::Max<i32>(0, (GetSystemMetrics(SM_CYSCREEN) - m_windowRect.Dimensions.y) / 2),
			m_windowRect.Dimensions.x,
			m_windowRect.Dimensions.y,
			nullptr,
			nullptr,
			appModule,
			this
		);

		Show();
		UpdateWindow(m_handle);
	}

	WinApiWindow::~WinApiWindow()
	{
		if (m_handle != nullptr)
		{
			DestroyWindow(m_handle);
		}
	}

	void WinApiWindow::Show()
	{
		ShowWindow(m_handle, SW_SHOW);
		SetFocus(m_handle);
		m_bIsFocused = true;
	}

	void WinApiWindow::Hide()
	{
		ShowWindow(m_handle, SW_HIDE);
		SetFocus(nullptr);
		m_bIsFocused = false;
	}

	bool WinApiWindow::PollEvents()
	{
		MSG msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				m_bIsFlaggedForClosing = true;
				return false;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}

		return true;
	}

	LRESULT CALLBACK WinApiWindow::WindowProc(HWND a_handle, u32 a_message, WPARAM a_wParam, LPARAM a_lParam)
	{
		WinApiWindow* windowInstance = NULL;

		switch (a_message)
		{
		case WM_CREATE:
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(a_lParam);
			windowInstance = (WinApiWindow*)pCreate->lpCreateParams;
			SetWindowLongPtr(a_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(windowInstance));
			break;
		}
		default:
			windowInstance = reinterpret_cast<WinApiWindow*>(GetWindowLongPtr(a_handle, GWLP_USERDATA));
			break;
		}

		return windowInstance != nullptr
			? windowInstance->LocalWindowProc(a_handle, a_message, a_wParam, a_lParam)
			: DefWindowProc(a_handle, a_message, a_wParam, a_lParam);
	}

	LRESULT CALLBACK WinApiWindow::LocalWindowProc(HWND a_handle, u32 a_message, WPARAM a_wParam, LPARAM a_lParam)
	{
		switch (a_message)
		{
			//#Todo: Add input parsing here!

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(a_handle, &ps);

			// All painting occurs here, between BeginPaint and EndPaint.
			FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
			EndPaint(a_handle, &ps);

			return 0;
		}
		//Called once the window regains focus.
		case WM_ACTIVATE:
		{
			m_bIsFocused = LOWORD(a_wParam) == WA_ACTIVE;
			return 0;
		}
		//The size of the window changed.
		case WM_SIZE:
		{
			m_windowRect.Width = LOWORD(a_lParam);
			m_windowRect.Height = HIWORD(a_lParam);

			if (a_wParam == SIZE_MINIMIZED)
			{
				m_bIsMinimized = true;
				m_bIsMaximized = false;
			}
			else if (a_wParam == SIZE_MAXIMIZED)
			{
				m_bIsMinimized = false;
				m_bIsMaximized = true;
			}
			else if (a_wParam == SIZE_RESTORED)
			{
				if (m_bIsMinimized) // Restoring from minimized state?
				{
					m_bIsMinimized = false;
				}
				else if (m_bIsMaximized) // Restoring from maximized state?
				{
					m_bIsMaximized = false;
				}
				else if (!m_bIsResizing)
				{
					//size move.
				}
			}

			return 0;
		}
		//Called when the window bars are hold onto to.
		case WM_ENTERSIZEMOVE:
		{
			m_bIsResizing = true;
			return 0;
		}
		//Triggered when the window bars are released.
		case WM_EXITSIZEMOVE:
		{
			m_bIsResizing = false;
			return 0;
		}

		//Disable alt+enter beeps!
		case WM_MENUCHAR:
		{
			return MAKELRESULT(0, MNC_CLOSE);
		}
		//Prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* const resizeInfo = reinterpret_cast<MINMAXINFO*>(a_lParam);
			resizeInfo->ptMinTrackSize = POINT(480, 270);
			return 0;
		}
		case WM_DESTROY:
		{
			m_bIsFlaggedForClosing = true;
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			return DefWindowProc(a_handle, a_message, a_wParam, a_lParam);
		}
		}
	}

}
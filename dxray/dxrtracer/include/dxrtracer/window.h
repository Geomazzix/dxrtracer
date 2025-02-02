#pragma once
#include <core/vath/vath.h>

namespace dxray
{
	struct WindowCreationInfo
	{
		String Title = String(PROJECT_NAME);
		vath::Rect<u32> Rect = vath::Rect<u32>(0, 0, 1600, 900);
	};

	/// <summary>
	/// The window class holds responsibility for the render surface properties 
	/// and the window itself.
	/// </summary>
	class WinApiWindow final
	{
	public:
		WinApiWindow(const WindowCreationInfo& a_createInfo);
		~WinApiWindow();

		void SetWindowTitle(const String& a_title);
		vath::Vector2u32 GetDimensions() const;
		u32 GetWidthInPx() const;
		u32 GetHeightInPx() const;
		bool IsMinimized() const;
		bool IsMaximized() const;
		bool PollEvents();
		void* GetNativeHandle() const;

	private:
		void Show();
		void Hide();

		static LRESULT CALLBACK WindowProc(HWND a_handle, u32 a_message, WPARAM a_wParam, LPARAM a_lParam);
		LRESULT CALLBACK LocalWindowProc(HWND a_handle, u32 a_message, WPARAM a_wParam, LPARAM a_lParam);

		String m_title;
		String m_className;

		HWND m_handle;
		vath::Rect<u32> m_windowRect;

		bool m_bIsMinimized;
		bool m_bIsMaximized;
		bool m_bIsFocused;
		bool m_bIsResizing;
		bool m_bIsFlaggedForClosing;
	};

	inline void WinApiWindow::SetWindowTitle(const String& a_title)
	{
		SetWindowText(m_handle, a_title.c_str());
	}

	inline vath::Vector2u32 WinApiWindow::GetDimensions() const
	{
		return m_windowRect.Dimensions;
	}

	inline u32 WinApiWindow::GetWidthInPx() const
	{
		return m_windowRect.Width;
	}

	inline u32 WinApiWindow::GetHeightInPx() const
	{
		return m_windowRect.Height;
	}

	inline bool WinApiWindow::IsMinimized() const
	{
		return m_bIsMinimized;
	}

	inline bool WinApiWindow::IsMaximized() const
	{
		return m_bIsMaximized;
	}

	inline void* WinApiWindow::GetNativeHandle() const
	{
		return m_handle;
	}
}
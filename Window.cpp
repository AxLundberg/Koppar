#include "Window.h"
#include <sstream>
#include "imgui_impl_win32.h"

Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept :
	hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = { 0 };
	// Window configuration
	wc.cbSize = sizeof( wc );
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

const wchar_t* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}
HINSTANCE Window::WindowClass::GetInstance() noexcept 
{
	return wndClass.hInst;
}

// Window
Window::Window(int width, int height, const wchar_t* name) 
	: width(width), height(height)
{
	//window size
	RECT wr = {};
	wr.left = 100;
	wr.right = width + wr.left;
	wr.top = 100;
	wr.bottom = height + wr.top;
	if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU, FALSE) == 0) throw LAST_EXCEPT();
	
	//throw Window::Exception(__LINE__, __FILEW__, hr);
	//throw HR_EXCEPT(ERROR_ARENA_TRASHED);
	
	//get hWnd, create and show window
	hWnd = CreateWindow( WindowClass::GetName(), name,
						 WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_MAXIMIZEBOX,
						 CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
						 nullptr, nullptr, WindowClass::GetInstance(), this );
	
	if (hWnd == 0) throw LAST_EXCEPT(); 

	ShowWindow( hWnd, SW_SHOWDEFAULT );
	// Init ImGui Win32 Impl
	ImGui_ImplWin32_Init(hWnd);
	// create gfx obj
	p_gfx = std::make_unique<Graphics>(hWnd, width, height);
}

// destructor to window
Window::~Window()
{ 
	ImGui_ImplWin32_Shutdown();
	DestroyWindow( hWnd );
}

// Set window title
void Window::SetWindowTitle(std::wstring title)
{
	SetWindowTextW(hWnd, title.c_str());
}

					/*initial message handler for window creation*/
LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// WM_NCCREATE first message sent when a window is created
	if (msg == WM_NCCREATE)
	{
		// lParam points to CREATESTRUCT structure containing info about the window being created
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
		// get ptr to window class from the creation data
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		// set data stored at the windows API side to the window
		// store pointer to windowclass inside the windows data on API side
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// set message proc to non-setup
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
		// pass set-up data to msg handler
		return pWnd->HandleMsg( hWnd, msg, wParam, lParam );
	}
	// handle with default handler if not WM_NCCREATE
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

				/*change from WINAPI call convention to C++ member call convention*/
LRESULT WINAPI Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	// get pointer to window class
	Window* const pWnd = reinterpret_cast<Window*>( GetWindowLongPtr(hWnd, GWLP_USERDATA) );
	// pass message to class handler
	return pWnd->HandleMsg( hWnd, msg, wParam, lParam );
}

void Window::ChangeCursor() noexcept
{
	cursor ^= true;
	
	if (!cursor)
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
		while (::ShowCursor(FALSE)>=0);

	}
	else
	{
		ClipCursor(nullptr);
		while (::ShowCursor(TRUE) < 0);
	}
}


std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT)
		{
			return (int)msg.wParam;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return {};
}

Graphics& Window::Gfx()
{
	return *p_gfx;
}


LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
	{
		return true;
	}
	const auto& imio = ImGui::GetIO();

	static std::wstring title;
	LPCWSTR sw = title.c_str();
	switch (msg)
	{
	// destroy window via destructor
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_CHAR:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
		if (static_cast<unsigned char>(wParam)==VK_ESCAPE)
		{
			PostQuitMessage(0);
			return 0;
		}
		keyboard.OnChar(static_cast<unsigned char>(wParam));
		break;
	case WM_KEYDOWN:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
		keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
		break;
	case WM_KEYUP:
		if (imio.WantCaptureKeyboard)
		{
			break;
		}
		keyboard.OnKeyReleased(static_cast<unsigned char>(wParam));
		break;
	case WM_MOUSEMOVE:
	{
		POINTS pts = MAKEPOINTS(lParam);
		if (!cursor)
		{
			spts.x = 600;
			spts.y = 450;
			ClientToScreen(hWnd, &spts);
			SetCursorPos(spts.x, spts.y);
		}
		moose.OnMouseMove(pts.x, pts.y);	
		break;
	}
	case WM_LBUTTONDOWN:
	{
		if (imio.WantCaptureMouse)
		{
			break;
		}
		POINTS pts = MAKEPOINTS(lParam);
		moose.OnLeftPressed(pts.x, pts.y);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		if (imio.WantCaptureMouse)
		{
			break;
		}
		POINTS pts = MAKEPOINTS(lParam);
		moose.OnRightPressed(pts.x, pts.y);
		break;
	}
	case WM_LBUTTONUP:
	{
		if (imio.WantCaptureMouse)
		{
			break;
		}
		POINTS pts = MAKEPOINTS(lParam);
		moose.OnLeftReleased(pts.x, pts.y);
		break;
	}
	case WM_RBUTTONUP:
	{
		if (imio.WantCaptureMouse)
		{
			break;
		}
		POINTS pts = MAKEPOINTS(lParam);
		moose.OnRightReleased(pts.x, pts.y);
		break;
	}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

					/*Exception handling*/
//constructor
Window::Exception::Exception(int line, const wchar_t* file, HRESULT hr)
	: ExceptionHandler(line, file), hr(hr)
{}


const wchar_t* Window::Exception::err_str() const noexcept
{
	std::wostringstream wostr;
	wostr << GetType() << std::endl << "ERROR: " << GetError() << std::endl
	   	 << "ERROR TEXT: " << GetErrorString() << std::endl << GetOriginString();
	errBuffer = wostr.str();
	return errBuffer.c_str();
}

const wchar_t* Window::Exception::GetType() const noexcept
{
	return L"WINDOW EXCEPTION";
}

//error getter
HRESULT Window::Exception::GetError() const noexcept
{
	return hr;
}

std::wstring Window::Exception::GetErrorString() const noexcept
{
	return ErrorToText(hr);
}

// Description string based on error code in HRESULT hr
std::wstring Window::Exception::ErrorToText(HRESULT hr) noexcept
{
	wchar_t* p_MsgBuff = nullptr;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				  nullptr,
		          hr,
	              MAKELANGID(LANG_NEUTRAL,
	   	          SUBLANG_DEFAULT),
		          reinterpret_cast<LPTSTR>(&p_MsgBuff),
		          0,
		          nullptr);
	std::wstring err_str = p_MsgBuff;
	LocalFree(p_MsgBuff);
	return err_str;
}
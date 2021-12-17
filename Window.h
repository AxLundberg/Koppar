#pragma once
#include "Winhead.h"
#include "ExceptionHandler.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Graphics.h"
#include <optional>
#include <sstream>
#include <iostream>
#include <string>
#include <memory>


class Window
{
public:
	class Exception : public ExceptionHandler
	{
	public:
		Exception(int line, const wchar_t* file, HRESULT hr);
		const wchar_t* err_str() const noexcept override; // override?
		const wchar_t* GetType() const noexcept override;
		HRESULT GetError() const noexcept;
		static std::wstring ErrorToText(HRESULT hr) noexcept;
		std::wstring GetErrorString() const noexcept;
	private:
		HRESULT hr;
	};
private: 
	class WindowClass
	{
	public:
		static wchar_t const* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass( const WindowClass& ) = delete;
		WindowClass& operator = (const WindowClass&) = delete;
		static constexpr const wchar_t* wndClassName = L"Engine window";
		static WindowClass wndClass;
		HINSTANCE hInst;
	};
public:
	Window(int width, int height, const wchar_t* name);
	~Window();
	Window(const Window&) = delete;
	Window& operator = (const Window&) = delete;
	void SetWindowTitle(std::wstring title);
	static std::optional<int> ProcessMessages() noexcept;
	void ChangeCursor() noexcept;
	Graphics& Gfx();
private:
	static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	static LRESULT WINAPI HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
private:
	int width;
	int height;
	HWND hWnd;
	std::unique_ptr<Graphics> p_gfx;
	//std::unique_ptr<ResourceFactory> p_resourceFactory;
	POINT spts;
public:
	bool cursor = true;
	Mouse moose;
	Keyboard keyboard;
};

#define HR_EXCEPT(hr) Window::Exception(__LINE__,__FILEW__,hr)
#define LAST_EXCEPT() Window::Exception(__LINE__,__FILEW__,GetLastError())
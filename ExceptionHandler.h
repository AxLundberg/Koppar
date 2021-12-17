#pragma once
#include <exception>
#include <string>

class ExceptionHandler : public std::exception
{
public:
	ExceptionHandler(int line, const wchar_t* file) noexcept;
	virtual const wchar_t* err_str() const noexcept; // override?
	virtual const wchar_t* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::wstring& GetFile() const noexcept;
	std::wstring GetOriginString() const noexcept;
private:
	int line;
	std::wstring file;
protected:
	mutable std::wstring errBuffer;
};
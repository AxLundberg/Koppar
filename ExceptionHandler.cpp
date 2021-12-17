#include "ExceptionHandler.h"
#include <sstream>

ExceptionHandler::ExceptionHandler(int line, const wchar_t* file) noexcept
	: line(line), file(file)
{}

const wchar_t* ExceptionHandler::err_str() const noexcept
{
	std::wostringstream wostr;
	wostr << GetType() << std::endl << GetOriginString();
	errBuffer = wostr.str();
	return errBuffer.c_str();
}

const wchar_t* ExceptionHandler::GetType() const noexcept
{
	return L"Exception Type";
}
const std::wstring& ExceptionHandler::GetFile() const noexcept
{
	return file;
}

int ExceptionHandler::GetLine() const noexcept 
{
	return line;
}

std::wstring ExceptionHandler::GetOriginString() const noexcept
{
	std::wostringstream wostr;
	wostr << "OCURRED IN FILE:  " << file << std::endl << "ON LINE:  " << line;
	return wostr.str();
}
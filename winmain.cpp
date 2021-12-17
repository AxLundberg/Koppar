#include "Window.h"
#include "Logic.h"
#include <sstream>

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR lpCmdLine,
    _In_ int nShowCmd)
{
    CoInitialize(NULL);
    try
    {
        return Logic{}.Run();
        /*Window wnd(800, 400, L"A window");
        MSG msg;
        BOOL gResult;
        while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            while ( !wnd.moose.IsEmpty() )
            {
                const auto e = wnd.moose.Read();
                if (e.GetType() == Mouse::Event::Type::Move)
                {
                    std::wostringstream wostr;
                    wostr << L"Mouse pos: ( " << e.GetPosX() << L", " << e.GetPosY() << L" )";
                    std::wstring tit = wostr.str();
                    wnd.SetWindowTitle(tit);
                }
                else if (e.GetType() == Mouse::Event::Type::LeftP)
                {
                    std::wostringstream wostr;
                    wostr << L"Mouse Left pressed at: ( " << e.GetPosX() << L", " << e.GetPosY() << L" )";
                    std::wstring tit = wostr.str();
                    wnd.SetWindowTitle(tit);
                }
                else if (e.GetType() == Mouse::Event::Type::RightP)
                {
                    std::wostringstream wostr;
                    wostr << L"Mouse Right pressed at: ( " << e.GetPosX() << L", " << e.GetPosY() << L" )";
                    std::wstring tit = wostr.str();
                    wnd.SetWindowTitle(tit);
                }
            }
        }

        if (gResult == -1)
        {
            return -1;
        }

        return msg.wParam;*/
    }
    catch (const ExceptionHandler& e)
    {
        MessageBox(nullptr,
                   e.err_str(),
                   e.GetType(),
                   MB_OK);
    }
    catch (const std::exception& ex)
    {
        MessageBox(nullptr,
            (LPCTSTR)ex.what(),
            L"standared exception",
            MB_OK);
    }
    catch (...)
    {
        MessageBox(nullptr,
            L"Don't know ::shrug::",
            L"Unknown exception",
            MB_OK);
    }
    return -1;
}

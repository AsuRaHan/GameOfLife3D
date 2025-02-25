#include "MainWindow.h"

MainWindow::MainWindow(HINSTANCE hInstance, int width, int height, int xPos, int yPos) :
    hInstance(hInstance), hWnd(NULL), pController(nullptr),
    windowWidth(width), windowHeight(height),
    windowPosX(xPos), windowPosY(yPos)
{
}

HWND MainWindow::Create() {

    // Проверка версии Windows
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((LPOSVERSIONINFO)&osvi);

    // Windows 8.1 = Major 6, Minor 3
    if (osvi.dwMajorVersion < 6 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion < 3)) {
        int result = MessageBox(NULL,
            L"Извините, но ваша версия Windows ниже 8.1. Это слишком мало для меня!\nХотите посетить сайт разработчика для поддержки?",
            L"Ошибка версии Windows",
            MB_YESNO | MB_ICONERROR);
        if (result == IDYES) {
            ShellExecute(NULL, L"open", L"https://yourwebsite.com", NULL, NULL, SW_SHOWNORMAL);
        }
        exit(1); // Завершаем создание окна
    }

    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc; // Используем статический метод
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(MainWindow*); // Добавляем пространство для указателя на MainWindow
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)); //LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"GameOfLife3DWindowClass";
    wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1)); //LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Создание окна
    hWnd = CreateWindowEx(
        0,
        L"GameOfLife3DWindowClass",
        L"Game of life 3D",
        WS_OVERLAPPEDWINDOW,
        windowPosX, windowPosY, windowWidth, windowHeight,
        NULL, NULL, hInstance, this // Передаем this как lpParam
    );
        
    if (!hWnd) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    RECT rcClient, rcWindow;
    GetClientRect(hWnd, &rcClient);
    GetWindowRect(hWnd, &rcWindow);

    // Вычисляем размеры рамок и заголовка
    int frameWidth = (rcWindow.right - rcWindow.left) - rcClient.right;
    int frameHeight = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;

    // Устанавливаем новые размеры, учитывая рамки и заголовок
    SetWindowPos(hWnd, NULL, windowPosX, windowPosY, windowWidth + frameWidth, windowHeight + frameHeight, SWP_NOMOVE | SWP_NOZORDER);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    return hWnd;
}

void MainWindow::SetController(WindowController* controller) {
    pController = controller;
}

HWND MainWindow::GetHwnd() const { return hWnd; }

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis;
    if (message == WM_NCCREATE) {
        // Извлекаем указатель на MainWindow из lParam при создании окна
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(pThis));
    }

    pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, 0));

    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    // Если ImGui не перехватил ввод, передаем его контроллеру
    if (pThis && pThis->pController) {
        // Проверка на активность любого элемента ImGui
        bool isInputActive = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive();
        // Обработка ввода для игры
        if (!isInputActive)
        {
            pThis->pController->HandleEvent(message, wParam, lParam);
        }
    }



    // Обработка оставшихся сообщений
    switch (message) {
    case WM_SIZE: // Обработка изменения размера
        if (pThis) {
            pThis->windowWidth = LOWORD(lParam);
            pThis->windowHeight = HIWORD(lParam);
        }
        break;
    case WM_MOVE:
    {
        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);
        pThis->windowPosX = windowRect.left;
        pThis->windowPosY = windowRect.top;
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


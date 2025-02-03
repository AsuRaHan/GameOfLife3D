#include "MainWindow.h"

MainWindow::MainWindow(HINSTANCE hInstance, int width, int height) :
    hInstance(hInstance), hWnd(NULL), pController(nullptr),
    windowWidth(width), windowHeight(height) {
}

bool MainWindow::Create() {
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
        return false;
    }

    // Создаем RECT для размеров окна
    RECT windowRect = { 0, 0, windowWidth, windowHeight };

    // Корректируем размеры с учетом рамки, заголовка и меню
    AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, TRUE, 0);

    // Создание окна с учетом скорректированных размеров
    hWnd = CreateWindowEx(
        0,
        L"GameOfLife3DWindowClass",
        L"Game of life 3D",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        NULL, NULL, hInstance, this // Передаем this как lpParam
    );
        
    if (!hWnd) {
        MessageBox(NULL, L"Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    return true;
}

void MainWindow::SetController(WindowController* controller) {
    pController = controller;
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        // Извлекаем указатель на MainWindow из lParam при создании окна
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        MainWindow* pThis = static_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, 0, reinterpret_cast<LONG_PTR>(pThis));
    }

    MainWindow* pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, 0));
    if (pThis) {
        if (pThis->pController) {
            pThis->pController->HandleEvent(message, wParam, lParam);
        }
    }

    switch (message) {
    case WM_SIZE: // Добавляем обработку изменения размера
    {
        pThis->windowWidth = LOWORD(lParam);
        pThis->windowHeight = HIWORD(lParam);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HWND MainWindow::GetHwnd() const { return hWnd; }
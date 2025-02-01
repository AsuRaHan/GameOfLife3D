#include "SettingsWindow.h"
#include <CommCtrl.h>

SettingsWindow::SettingsWindow(HINSTANCE hInstance) : hWnd(NULL), hInstance(hInstance) {
    // ����������� ������ ����
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"SettingsWindowClass";
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
    }
}

SettingsWindow::~SettingsWindow() {
    // ����������� ���� ��� �������� �������
    if (hWnd) {
        DestroyWindow(hWnd);
    }
}

bool SettingsWindow::Create(HWND hParentWnd) {
    hWnd = CreateWindowEx(
        0,
        L"SettingsWindowClass",
        L"��������� ����",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        hParentWnd, // ������������� ������������ ����
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) {
        MessageBox(NULL, L"Settings Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    // ���������� ��������� ���������� (������)
    CreateWindowEx(0, L"BUTTON", L"���������", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 10, 80, 25, hWnd, (HMENU)1, hInstance, NULL);
    CreateWindowEx(0, L"BUTTON", L"���������", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 100, 10, 80, 25, hWnd, (HMENU)2, hInstance, NULL);

    // ������������� common controls
    InitCommonControls();

    return true;
}

void SettingsWindow::Show() {
    ShowWindow(hWnd, SW_SHOW);
}

void SettingsWindow::Hide() {
    ShowWindow(hWnd, SW_HIDE);
}

LRESULT CALLBACK SettingsWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // ID ��� ������ "���������"
            // ����� ����� ������� ����� ��� �������� ����
            MessageBox(hWnd, L"������� �������� �� �����������", L"Info", MB_OK);
            break;
        case 2: // ID ��� ������ "���������"
            // ����� ����� ������� ����� ��� ���������� ����
            MessageBox(hWnd, L"������� ���������� �� �����������", L"Info", MB_OK);
            break;
        }
        break;
    case WM_CLOSE:
        // �������� ���� ������ ��� ��������, ����� ����� ���� ����� ��� ��������
        ShowWindow(hWnd, SW_HIDE);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
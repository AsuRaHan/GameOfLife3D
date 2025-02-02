#include "SettingsWindow.h"
#include <CommCtrl.h>

SettingsWindow::SettingsWindow(HINSTANCE hInstance) : hWnd(NULL), hInstance(hInstance) {
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
        CW_USEDEFAULT, CW_USEDEFAULT, 450, 400,
        hParentWnd,
        NULL,
        hInstance,
        NULL
    );

    if (!hWnd) {
        MessageBox(NULL, L"Settings Window Creation Failed!", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    InitCommonControls();

    int yPos = 10;

    // ������ �����
    CreateLabel(hWnd, 10, yPos, 100, 20, L"������:");
    HWND editWidth = CreateEdit(hWnd, 120, yPos, 150, 20, 1);
    //SetWindowText(editWidth, L"100");
    yPos += 30;

    // ������ �����
    CreateLabel(hWnd, 10, yPos, 100, 20, L"������:");
    HWND editHeight = CreateEdit(hWnd, 120, yPos, 150, 20, 2);
    //SetWindowText(editHeight, L"100");
    yPos += 30;

    // ������ ��� ������� ������� �����
    CreateButton(hWnd, 10, yPos, 150, 25, 9, L"������");
    yPos += 35;

    // �������� ���������
    CreateLabel(hWnd, 10, yPos, 100, 20, L"��������:");
    CreateSlider(hWnd, 120, yPos + 5, 200, 20, 3, 1, 1000);
    yPos += 40;

    // ������� ����
    CreateLabel(hWnd, 10, yPos, 100, 20, L"�������:");
    CreateEdit(hWnd, 120, yPos + 5, 310, 20, 4);
    yPos += 40;

    // ���� ����� ������
    CreateLabel(hWnd, 10, yPos, 150, 20, L"���� ����� ������:");
    CreateButton(hWnd, 170, yPos, 150, 20, 5, L"�������");
    yPos += 30;

    // ���� ������� ������
    CreateLabel(hWnd, 10, yPos, 150, 20, L"���� ������� ������:");
    CreateButton(hWnd, 170, yPos, 150, 20, 6, L"�������");
    yPos += 40;

    // �������� � ����������
    CreateButton(hWnd, 10, yPos, 155, 25, 7, L"���������");
    CreateButton(hWnd, 185, yPos, 155, 25, 8, L"���������");
    yPos += 35;

    // �������� � ���������� ��� CSV
    CreateButton(hWnd, 10, yPos, 155, 25, 10, L"��������� ��� CSV");
    CreateButton(hWnd, 185, yPos, 155, 25, 11, L"��������� ��� CSV");

    return true;
}

void SettingsWindow::Show() {
    ShowWindow(hWnd, SW_SHOW);
}

void SettingsWindow::Hide() {
    ShowWindow(hWnd, SW_HIDE);
}

void SettingsWindow::CreateLabel(HWND hParent, int x, int y, int width, int height, const wchar_t* text) {
    CreateWindowEx(0, L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y, width, height, hParent, NULL, hInstance, NULL);
}

HWND SettingsWindow::CreateButton(HWND hParent, int x, int y, int width, int height, int id, const wchar_t* text) {
    return CreateWindowEx(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, x, y, width, height, hParent, (HMENU)id, hInstance, NULL);
}

HWND SettingsWindow::CreateEdit(HWND hParent, int x, int y, int width, int height, int id) {
    return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, x, y, width, height, hParent, (HMENU)id, hInstance, NULL);
}

void SettingsWindow::CreateSlider(HWND hParent, int x, int y, int width, int height, int id, int min, int max) {
    HWND slider = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ, x, y, width, height, hParent, (HMENU)id, hInstance, NULL);
    SendMessage(slider, TBM_SETRANGE, TRUE, MAKELONG(min, max));
    SendMessage(slider, TBM_SETPOS, TRUE, 1000); // ������������� ��������� ��������
}

LRESULT CALLBACK SettingsWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1: // ������ �����
            MessageBox(hWnd, L"��������� ������ �����", L"Info", MB_OK);
            break;
        case 2: // ������ �����
            MessageBox(hWnd, L"��������� ������ �����", L"Info", MB_OK);
            break;
        case 5: // ���� ����� ������
            MessageBox(hWnd, L"����� ����� ����� ������", L"Info", MB_OK);
            break;
        case 6: // ���� ������� ������
            MessageBox(hWnd, L"����� ����� ������� ������", L"Info", MB_OK);
            break;
        case 7: // ��������
            MessageBox(hWnd, L"�������� �����", L"Info", MB_OK);
            break;
        case 8: // ����������
            MessageBox(hWnd, L"���������� �����", L"Info", MB_OK);
            break;
        case 9: // ������ ������ �����
            MessageBox(hWnd, L"������ ������ �����", L"Info", MB_OK);
            break;
        case 10: // ��������� ��� CSV
            MessageBox(hWnd, L"�������� ��� CSV", L"Info", MB_OK);
            break;
        case 11: // ��������� ��� CSV
            MessageBox(hWnd, L"���������� ��� CSV", L"Info", MB_OK);
            break;
        }
        break;

    case WM_HSCROLL:
    {
        int pos = LOWORD(wParam); // �������� ������� �������
        if ((HWND)lParam == GetDlgItem(hWnd, 3)) { // ���������, ��� ��������� �� ������ ��������
            switch (HIWORD(wParam)) {
            case TB_THUMBTRACK:
                MessageBox(hWnd, L"������� �������", L"Info", MB_OK);
                break;
            case TB_ENDTRACK:
                MessageBox(hWnd, L"������� �������", L"Info", MB_OK);
                break;
            }
        }
    }
    break;

    case WM_CLOSE:
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
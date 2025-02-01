#pragma once
#include <Windows.h>

class SettingsWindow {
private:
    HWND hWnd;
    HINSTANCE hInstance;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    SettingsWindow(HINSTANCE hInstance);
    ~SettingsWindow();
    bool Create(HWND hParentWnd);
    void Show();
    void Hide();
};
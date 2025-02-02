#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include <string>

class SettingsWindow {
private:
    HWND hWnd;
    HINSTANCE hInstance;
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    void CreateLabel(HWND hParent, int x, int y, int width, int height, const wchar_t* text);
    HWND CreateButton(HWND hParent, int x, int y, int width, int height, int id, const wchar_t* text);
    HWND CreateEdit(HWND hParent, int x, int y, int width, int height, int id);
    void CreateSlider(HWND hParent, int x, int y, int width, int height, int id, int min, int max);

public:
    SettingsWindow(HINSTANCE hInstance);
    ~SettingsWindow();
    bool Create(HWND hParentWnd);
    void Show();
    void Hide();
};
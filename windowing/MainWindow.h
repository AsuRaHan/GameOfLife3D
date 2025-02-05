#pragma once
#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <Windows.h>
#include "../resource.h"
#include "WindowController.h"
//#include "system/OpenGLInitializer.h"

class MainWindow {
private:
    HWND hWnd;
    HINSTANCE hInstance;
    WindowController* pController;
    int windowWidth; // ���� ��� ������
    int windowHeight; // ���� ��� ������
public:
    MainWindow(HINSTANCE hInstance, int width = 1000, int height = 1000);
    bool Create();
    void SetController(WindowController* controller);
    HWND GetHwnd() const;
    // ������� ��� �������� ����
    int GetWidth() const { return windowWidth; }
    int GetHeight() const { return windowHeight; }

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
#endif // MAINWINDOW_H_
#pragma once
#ifndef UICONTROLLER_H_
#define UICONTROLLER_H_

#include "../game/GameController.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

class UIController {
private:
    GameController* gameController;

    bool showExitDialog;
    const char* aboutText = R"(
����� ���������� � ������� �� ������������� ���������!
    ����� �� ������ ����� ���������� �� ������ � ���������� ���������:
    - ������ ���������: ������� ������ '������ ���������' ��� ������� �����.
    - ���������� ���������: ������� '���������� ���������', ����� ������������� �������.
    - ��� ���������: ����������� '��� ���������' ��� ������� ����������� �� ���� ���������.
    - �������� ����: ������ '�������� ����' ������� ��� �����.
    - ��������� ������: '��������� ������' ��������� ����� ��������� �������.
    - ���������� ������: ����������� '�������� �����' ��� '�������� �����' ��� ��������� ��������� �����.
    - ����������/��������: '���������' � '���������' ��������� �������� � ����������� ����.
    "��� ������ �� ��������� ������� '�����'.
)";

public:
    UIController(GameController* gc);
    void InitializeUI();
    void DrawUI();
    void DrawMenuBar();
    void UpdateUIState();
};

#endif // UICONTROLLER_H_
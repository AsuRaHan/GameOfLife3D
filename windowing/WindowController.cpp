#include "WindowController.h"
#include "MainWindow.h"

#include <iostream>

WindowController::WindowController(MainWindow* window, Renderer* renderer, GameController* gameController)
    : pWindow(window), pRenderer(renderer), pGameController(gameController),
    mouseCaptured(false), lastMouseX(0), lastMouseY(0) {

    // ��������� ������� ��� ���������� ���������
    timerId = SetTimer(pWindow->GetHwnd(), 1, 100, NULL); // �������� 100 ��
    if (!timerId) {
        MessageBox(NULL, L"Timer could not be set", L"Error", MB_OK | MB_ICONERROR);
    }
}

WindowController::~WindowController() {
    if (timerId) {
        KillTimer(pWindow->GetHwnd(), timerId);
    }
}

void WindowController::Resize(int width, int height) {
    if (pRenderer) {
        pRenderer->OnWindowResize(width, height);
    }
}

void WindowController::HandleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_TIMER:
        if (wParam == timerId) {
            pGameController->update(); // ��������� ���������
        }
        break;

    case WM_SIZE:
        Resize(LOWORD(lParam), HIWORD(lParam)); // ��������� ��������� �������� ����
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_SPACE: // ������ ��� �������/��������� ���������
            if (pGameController->isSimulationRunning()) {
                pGameController->stopSimulation();
            }
            else {
                pGameController->startSimulation();
            }
            break;

        case VK_RIGHT: // ������� ������ ��� ���������� ���������
            if (!pGameController->isSimulationRunning()) {
                pGameController->stepSimulation(); // ������� � ���������� ���������
            }
            break;

        case VK_LEFT: // ������� ����� ��� ����������� ���������
            if (!pGameController->isSimulationRunning()) {
                pGameController->previousGeneration(); // ������� � ����������� ���������
            }
            break;

        }
        break;

    case WM_MOUSEWHEEL:
        if (pRenderer) {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            float moveZ = zDelta > 0 ? -1.1f : 1.1f;
            Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
            camera.Move(0.0f, 0.0f, moveZ);
        }
        break;

    case WM_LBUTTONDOWN: // ����� ���� ����
        HandleMouseClick(LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_RBUTTONDOWN: // ������ ������� ����
        SetCapture(pWindow->GetHwnd());
        mouseCaptured = true;
        lastMouseX = LOWORD(lParam);
        lastMouseY = HIWORD(lParam);
        // ������ ������
        //ShowCursor(FALSE);
        break;

    case WM_RBUTTONUP: // ����� ������� ����
        ReleaseCapture();
        mouseCaptured = false;
        // �������� ������
        //ShowCursor(TRUE);
        break;

    case WM_MOUSEMOVE:
        if (mouseCaptured && pRenderer) {
            int currentX = LOWORD(lParam);
            int currentY = HIWORD(lParam);

            float dx = static_cast<float>(currentX - lastMouseX) * 0.1f; // ���������������� ��� �����������
            float dy = static_cast<float>(currentY - lastMouseY) * 0.1f;

            // ���������� ������
            Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
            camera.Move(dx, dy, 0.0f); // �������� �������� �� ���� ��� Y

            lastMouseX = currentX;
            lastMouseY = currentY;
        }
        break;

    default:
        break;
    }
}

void WindowController::HandleMouseClick(int screenX, int screenY) {
    if (pGameController && pRenderer) {
        float cellSize = pGameController->getCellSize();
        int gridWidth = pGameController->getGridWidth();
        int gridHeight = pGameController->getGridHeight();

    }
}
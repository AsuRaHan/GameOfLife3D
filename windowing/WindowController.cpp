#include "WindowController.h"
#include "MainWindow.h"

#include <iostream>

WindowController::WindowController(MainWindow* window, Renderer* renderer, GameController* gameController)
    : pWindow(window), pRenderer(renderer), pGameController(gameController),
    mouseCaptured(false), lastMouseX(0), lastMouseY(0), isMiddleButtonDown(false),
    gridPicker(renderer->GetCamera(), *gameController) {

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
        case 'R': // ������� 'R' ��� ���������� ���������� ����
            if (!pGameController->isSimulationRunning()) {
                pGameController->randomizeGrid(0.5f); // ��������� ���������� ����
            }
            break;
        case 'W':
            MoveCamera(0.0f, 1.0f); // ������� ������
            break;
        case 'S':
            MoveCamera(0.0f, -1.0f); // ������� �����
            break;
        case 'A':
            MoveCamera(-1.0f, 0.0f); // ������� �����
            break;
        case 'D':
            MoveCamera(1.0f, 0.0f); // ������� ������
            break;
        case 'T':
            ResetCamera();
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
    case WM_MBUTTONDOWN: // ������� ������� ������ ����
        SetCapture(pWindow->GetHwnd());
        isMiddleButtonDown = true;
        lastMouseX = LOWORD(lParam);
        lastMouseY = HIWORD(lParam);
        break;

    case WM_MBUTTONUP: // ���������� ������� ������ ����
        ReleaseCapture();
        isMiddleButtonDown = false;
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
        //if (isMiddleButtonDown && pRenderer) {
        //    int currentX = LOWORD(lParam);
        //    int currentY = HIWORD(lParam);

        //    float dx = static_cast<float>(currentX - lastMouseX) * 0.005f; // ���������������� ��� �������� �� YAW
        //    float dy = static_cast<float>(currentY - lastMouseY) * 0.005f; // ���������������� ��� �������� �� PITCH

        //    RotateCamera(dx, dy);

        //    lastMouseX = currentX;
        //    lastMouseY = currentY;
        //}
        break;

    default:
        break;
    }
}

void WindowController::HandleMouseClick(int screenX, int screenY) {
    if (pGameController && pRenderer) {
        gridPicker.HandleClick(screenX, screenY, pRenderer->getWindowWidth(), pRenderer->getWindowHeight());
    }
}

void WindowController::RotateCamera(float yaw, float pitch) {
    if (pRenderer) {
        Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
        camera.Rotate(yaw, pitch);
    }
}

void WindowController::MoveCamera(float dx, float dy) {
    if (pRenderer) {
        Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
        // �������� ������� ����������� ������ ��� ���������� ��������
        float dirX, dirY, dirZ;
        camera.GetDirection(dirX, dirY, dirZ);

        // ������� ������ ������/����� �� ����������� �������
        float moveX = dirX * dy;
        float moveZ = dirZ * dy;

        // ������� ������ �����/������ ��������������� ����������� �������
        float rightX = dirZ;
        float rightZ = -dirX;
        moveX += rightX * dx;
        moveZ += rightZ * dx;

        // ��������� ��������
        camera.Move(moveX, 0.0f, moveZ);
    }
}

void WindowController::ResetCamera() {
    if (pRenderer && pGameController) {
        Camera& camera = const_cast<Camera&>(pRenderer->GetCamera());
        float width = pGameController->getGridWidth() * pGameController->getCellSize();
        float height = pGameController->getGridHeight() * pGameController->getCellSize();

        // ���������� ����������� zoom, ����� ��� ���� ���� �����
        float windowAspectRatio = static_cast<float>(pRenderer->getWindowWidth()) / pRenderer->getWindowHeight();
        float fieldAspectRatio = width / height;

        float zoom = 1.0f;
        if (fieldAspectRatio > windowAspectRatio) {
            zoom = width / (2.0f * std::tan(camera.GetFOV() / 2.0f * 3.14159265358979323846f / 180.0f) * pRenderer->getWindowWidth() / pRenderer->getWindowHeight());
        }
        else {
            zoom = height / (2.0f * std::tan(camera.GetFOV() / 2.0f * 3.14159265358979323846f / 180.0f));
        }

        // ������������� ������� ������
        camera.SetPosition(width / 2.0f, height / 2.0f, zoom);
        camera.SetDirection(0.0f, 0.0f, -1.0f); // ����������� ����� ��� Z (�����)
        camera.SetUpVector(0.0f, 1.0f, 0.0f); // ��������� ������� "�����"
    }
}
#include "WindowController.h"
#include "MainWindow.h"

#include <iostream>

WindowController::WindowController(MainWindow* window, Renderer* renderer, GameController* gameController)
    : pWindow(window), pRenderer(renderer), pGameController(gameController),
    mouseCaptured(false), lastMouseX(0), lastMouseY(0), isMiddleButtonDown(false),
    gridPicker(renderer->GetCamera()) {
    ResetCamera();
}

WindowController::~WindowController() {

}


void WindowController::Resize(int width, int height) {
    if (pRenderer) {
        pRenderer->OnWindowResize(width, height);
    }
}

void WindowController::HandleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {

    case WM_SIZE:
        Resize(LOWORD(lParam), HIWORD(lParam)); // ��������� ��������� �������� ����
        break;

    case WM_KEYDOWN:
        if (wParam >= '1' && wParam <= '9') {
            pGameController->setCurrentPattern(wParam - '0');
        }
        switch (wParam) {
            break;
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
        case VK_ADD: // ������� '+' �� �������
        case VK_OEM_PLUS: // ������� '+' �� �������� ����������
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                simulationSpeedMultiplier -= 0.25f; // ��������� �������� �� 250 ��
            }
            else {
                simulationSpeedMultiplier -= 0.025f; // ��������� �������� �� 25 ��
            }
            if (simulationSpeedMultiplier < 0.01f) {
                simulationSpeedMultiplier = 0.01f; // ������������� ������� 10 ��
            }
            if (pGameController->isSimulationRunning()) {
                pGameController->setSimulationSpeed(simulationSpeedMultiplier);
            }
            break;

        case VK_SUBTRACT: // ������� '-' �� �������
        case VK_OEM_MINUS: // ������� '-' �� �������� ����������
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                simulationSpeedMultiplier += 0.25f; // ����������� �������� �� 250 ��
            }
            else {
                simulationSpeedMultiplier += 0.025f; // ����������� �������� �� 25 ��
            }
            if (simulationSpeedMultiplier > 0.25f) {
                simulationSpeedMultiplier = 0.25f; // ������������� �������� 250 ��
            }
            if (pGameController->isSimulationRunning()) {
                pGameController->setSimulationSpeed(simulationSpeedMultiplier);
            }
            break;

        case 'R':
            if (!pGameController->isSimulationRunning()) {
                pGameController->randomizeGrid(0.1f); // ��������� ���������� ����
            }
            break;
        case 'W':
            MoveCamera(0.0f, 5.0f); // ������� ������
            break;
        case 'S':
            MoveCamera(0.0f, -5.0f); // ������� �����
            break;
        case 'A':
            MoveCamera(-5.0f, 0.0f); // ������� �����
            break;
        case 'D':
            MoveCamera(5.0f, 0.0f); // ������� ������
            break;
        case 'T':
            ResetCamera();
            break;
        case 'C':
            if (!pGameController->isSimulationRunning()) {
                pGameController->clearGrid(); // �������� ����. ����� ����
            }
            break;
        case 'Y':
            if (!pGameController->isSimulationRunning()) {
                pGameController->setWoldToroidal(!pGameController->getWoldToroidal()); // �������� ����. ����� ����
            }
            break;
        case 'I':
            if (!pGameController->isSimulationRunning()) {
                pGameController->initializeGrid(); // �������� ����. ����� ����
            }
            break;
        case 'L':
            if (!pGameController->isSimulationRunning()) {
                pGameController->resizeGrid(30, 30);
                pRenderer->RebuildGameField();
            }
            break;
        case 'G':
            pRenderer->setShowGrid(!pRenderer->getShowGrid());
            break;

        }
        break;

    case WM_MOUSEWHEEL:
        if (pRenderer) {
            short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            float moveZ = zDelta > 0 ? -1.1f : 1.1f;
            // �������� ��������� ������� Ctrl
            if (GetKeyState(VK_CONTROL) & 0x8000) {
                moveZ *= 10.0f; // ����������� �������� ���� � 10 ���
            }
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
        PlacePattern(LOWORD(lParam), HIWORD(lParam));
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
        break;

    default:
        break;
    }
}

void WindowController::HandleMouseClick(int screenX, int screenY) {
    float worldX, worldY;
    gridPicker.ScreenToWorld(screenX, screenY, pRenderer->getWindowWidth(), pRenderer->getWindowHeight(), worldX, worldY);

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    int x = static_cast<int>(worldX / cellSize);
    int y = static_cast<int>(worldY / cellSize);

    if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
        pGameController->toggleCellState(x, y);
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

void WindowController::PlacePattern(int screenX, int screenY) {
    float worldX, worldY;
    gridPicker.ScreenToWorld(screenX, screenY, pRenderer->getWindowWidth(), pRenderer->getWindowHeight(), worldX, worldY);

    int gridWidth = pGameController->getGridWidth();
    int gridHeight = pGameController->getGridHeight();
    float cellSize = pGameController->getCellSize();

    // ����������� ������� ���������� � ���������� �����
    int x = static_cast<int>(worldX / cellSize);
    int y = static_cast<int>(worldY / cellSize);

    // ��� ���������� �������, ���������, ��� �� �� ������� �� ������� �����
    if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
        pGameController->PlacePattern(x, y);
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
